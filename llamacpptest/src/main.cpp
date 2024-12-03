#include <llama.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <cmath>

std::vector<std::string> split_lines(const std::string& s, const std::string& separator = "\n")
{
    std::vector<std::string> lines;
    size_t start = 0;
    size_t end = s.find(separator);
    while(end != std::string::npos) {
        lines.push_back(s.substr(start, end - start));
        start = end + separator.length();
        end = s.find(separator, start);
    }
    lines.push_back(s.substr(start)); // Add the last part
    return lines;
}

std::vector<llama_token> common_tokenize(
    const struct llama_model* model,
    const std::string& text,
    bool add_special,
    bool parse_special)
{
    // upper limit for the number of tokens
    int n_tokens = text.length() + 2 * add_special;
    std::vector<llama_token> result(n_tokens);
    n_tokens = llama_tokenize(model, text.data(), text.length(), result.data(), result.size(), add_special, parse_special);
    if(n_tokens < 0) {
        result.resize(-n_tokens);
        int check = llama_tokenize(model, text.data(), text.length(), result.data(), result.size(), add_special, parse_special);
        GGML_ASSERT(check == -n_tokens);
    } else {
        result.resize(n_tokens);
    }
    return result;
}

void common_batch_add(
    llama_batch& batch,
    llama_token id,
    llama_pos pos,
    const std::vector<llama_seq_id>& seq_ids,
    bool logits)
{
    GGML_ASSERT(batch.seq_id[batch.n_tokens] && "llama_batch size exceeded");

    batch.token[batch.n_tokens] = id;
    batch.pos[batch.n_tokens] = pos;
    batch.n_seq_id[batch.n_tokens] = seq_ids.size();
    for(size_t i = 0; i < seq_ids.size(); ++i) {
        batch.seq_id[batch.n_tokens][i] = seq_ids[i];
    }
    batch.logits[batch.n_tokens] = logits;

    batch.n_tokens++;
}

void batch_add_seq(llama_batch & batch, const std::vector<llama_token> & tokens, llama_seq_id seq_id)
{
    for (size_t i = 0; i < tokens.size(); ++i) {
        common_batch_add(batch, tokens[i], i, { seq_id }, true);
    }
}

void common_batch_clear(llama_batch& batch)
{
    batch.n_tokens = 0;
}

void common_embd_normalize(const float * inp, float * out, int32_t n, int32_t embd_norm)
{
    double sum = 0.0;
    switch (embd_norm) {
        case -1: // no normalisation
            sum = 1.0;
            break;
        case 0: // max absolute
            for (int i = 0; i < n; i++) {
                if (sum < std::abs(inp[i])) sum = std::abs(inp[i]);
            }
            sum /= 32760.0; // make an int16 range
            break;
        case 2: // euclidean
            for (int i = 0; i < n; i++) {
                sum += inp[i] * inp[i];
            }
            sum = std::sqrt(sum);
            break;
        default: // p-norm (euclidean is p-norm p=2)
            for (int i = 0; i < n; i++) {
                sum += std::pow(std::abs(inp[i]), embd_norm);
            }
            sum = std::pow(sum, 1.0 / embd_norm);
            break;
    }

    const float norm = sum > 0.0 ? 1.0 / sum : 0.0f;

    for (int i = 0; i < n; i++) {
        out[i] = inp[i] * norm;
    }
}

float common_embd_similarity_cos(const float * embd1, const float * embd2, int n)
{
    double sum  = 0.0;
    double sum1 = 0.0;
    double sum2 = 0.0;

    for (int i = 0; i < n; i++) {
        sum  += embd1[i] * embd2[i];
        sum1 += embd1[i] * embd1[i];
        sum2 += embd2[i] * embd2[i];
    }

    // Handle the case where one or both vectors are zero vectors
    if (sum1 == 0.0 || sum2 == 0.0) {
        if (sum1 == 0.0 && sum2 == 0.0) {
            return 1.0f; // two zero vectors are similar
        }
        return 0.0f;
    }

    return sum / (sqrt(sum1) * sqrt(sum2));
}

void batch_decode(llama_context* ctx, llama_batch& batch, float* output, int32_t n_seq, int32_t n_embd, int32_t embd_norm)
{
    const enum llama_pooling_type pooling_type = llama_pooling_type(ctx);
    const struct llama_model * model = llama_get_model(ctx);

    // clear previous kv_cache values (irrelevant for embeddings)
    llama_kv_cache_clear(ctx);

    // run model
    fprintf(stderr, "%s: n_tokens = %d, n_seq = %d\n", __func__, batch.n_tokens, n_seq);
    if (llama_model_has_encoder(model) && !llama_model_has_decoder(model)) {
        // encoder-only model
        if (llama_encode(ctx, batch) < 0) {
            fprintf(stderr, "%s : failed to encode\n", __func__);
        }
    } else if (!llama_model_has_encoder(model) && llama_model_has_decoder(model)) {
        // decoder-only model
        if (llama_decode(ctx, batch) < 0) {
            fprintf(stderr, "%s : failed to decode\n", __func__);
        }
    }

    for (int i = 0; i < batch.n_tokens; i++) {
        if (!batch.logits[i]) {
            continue;
        }

        const float * embd = nullptr;
        int embd_pos = 0;

        if (pooling_type == LLAMA_POOLING_TYPE_NONE) {
            // try to get token embeddings
            embd = llama_get_embeddings_ith(ctx, i);
            embd_pos = i;
            GGML_ASSERT(embd != NULL && "failed to get token embeddings");
        } else {
            // try to get sequence embeddings - supported only when pooling_type is not NONE
            embd = llama_get_embeddings_seq(ctx, batch.seq_id[i][0]);
            embd_pos = batch.seq_id[i][0];
            GGML_ASSERT(embd != NULL && "failed to get sequence embeddings");
        }

        float * out = output + embd_pos * n_embd;
        common_embd_normalize(embd, out, n_embd, embd_norm);
    }
}

int main(void)
{
    llama_backend_init();
    llama_model_params model_params = {};
    model_params.n_gpu_layers = 0;
    model_params.split_mode = LLAMA_SPLIT_MODE_LAYER;
    model_params.main_gpu = 0;
    model_params.tensor_split = nullptr;
    model_params.rpc_servers = nullptr;
    model_params.kv_overrides = nullptr;
    model_params.vocab_only = false;
    model_params.use_mmap = false;
    model_params.use_mlock = true;
    model_params.check_tensors = true;

    llama_model* model = llama_load_model_from_file("./data/multilingual-e5-base-Q8_0.gguf", model_params);
    if(nullptr == model) {
        return -1;
    }
    llama_context_params context_params = {};
    context_params.n_ctx = 0;
    context_params.n_batch = 2048;
    context_params.n_ubatch = 512;
    context_params.n_seq_max = 1;
    context_params.n_threads = 1;
    context_params.n_threads_batch = 1;
    context_params.rope_scaling_type = LLAMA_ROPE_SCALING_TYPE_UNSPECIFIED;
    context_params.pooling_type = LLAMA_POOLING_TYPE_UNSPECIFIED;
    context_params.attention_type = LLAMA_ATTENTION_TYPE_UNSPECIFIED;

    context_params.rope_freq_base = 0.0f;
    context_params.rope_freq_scale = 0.0f;
    context_params.yarn_ext_factor = -1.0f;
    context_params.yarn_attn_factor = 1.0f;
    context_params.yarn_beta_fast = 32.0f;
    context_params.yarn_beta_slow = 1.0f;
    context_params.yarn_orig_ctx = 0;
    context_params.defrag_thold = 0.1f;

    context_params.cb_eval = nullptr;
    context_params.cb_eval_user_data = nullptr;

    context_params.type_k = GGML_TYPE_F16;
    context_params.type_v = GGML_TYPE_F16;

    context_params.logits_all = false;
    context_params.embeddings = true;
    context_params.offload_kqv = true;
    context_params.flash_attn = false;
    context_params.no_perf = false;

    context_params.abort_callback = nullptr;
    context_params.abort_callback_data = nullptr;

    llama_context* context = llama_new_context_with_model(model, context_params);
    if(llama_model_has_encoder(model) && llama_model_has_decoder(model)) {
        fprintf(stderr, "%s: computing embeddings in encoder-decoder models is not supported\n", __func__);
        return -1;
    }
    if(llama_n_ctx_train(model) < llama_n_ctx(context)) {
        fprintf(stderr, "%s: warning: model was trained on only %d context tokens (%d specified)\n", __func__, llama_n_ctx_train(model), llama_n_ctx(context));
    }
    std::string prompt =
        "Transcript of a dialog, where the User interacts with an Assistant named Bob. Bob is helpful, kind, honest, good at writing, and never fails to answer the User's requests immediately and with precision.\n"
        "Hello, Bob.\n"
        "Hello. How may I help you today?";

    // split the prompt into lines
    std::vector<std::string> prompts = split_lines(prompt);

    // max batch size
    const uint32_t n_batch = context_params.n_batch;
    GGML_ASSERT(context_params.n_batch >= context_params.n_ctx);

    // tokenize the prompts and trim
    std::vector<std::vector<int32_t>> inputs;
    for (const auto & prompt : prompts) {
        std::vector<llama_token> input = common_tokenize(model, prompt, true, true);
        if (input.size() > n_batch) {
            fprintf(stderr, "%s: number of tokens in input line (%lld) exceeds batch size (%lld), increase batch size and re-run\n",
                    __func__, (long long int) input.size(), (long long int) n_batch);
            return -1;
        }
        inputs.push_back(std::move(input));
    }

    for (const std::vector<llama_token>& input : inputs) {
        if (input.empty() || input.back() != llama_token_sep(model)) {
            fprintf(stderr, "%s: last token in the prompt is not SEP\n", __func__);
            fprintf(stderr, "%s: 'tokenizer.ggml.add_eos_token' should be set to 'true' in the GGUF header\n", __func__);
        }
    }

    llama_batch batch = llama_batch_init(n_batch, 0, 1);
    size_t n_embd_count = 0;
    if(llama_pooling_type(context) == LLAMA_POOLING_TYPE_NONE){
        for(size_t k=0; k<inputs.size(); ++k){
            n_embd_count += inputs[k].size();
        }
    }else{
        n_embd_count = inputs.size();
    }
    const int32_t n_embd = llama_n_embd(model);
    std::vector<float> embeddings(n_embd_count*n_embd, 0);

    float* emb = embeddings.data();
    // break into batches
    int32_t e = 0; // number of embeddings already stored
    int32_t s = 0; // number of prompts in current batch
    for (size_t k = 0; k < prompts.size(); ++k) {
        // clamp to n_batch tokens
        const std::vector<llama_token>& input = inputs[k];

        const uint64_t n_toks = input.size();
        // encode if at capacity
        if (batch.n_tokens + n_toks > n_batch) {
            float * out = emb + e * n_embd;
            batch_decode(context, batch, out, s, n_embd, 2);
            e += llama_pooling_type(context) == LLAMA_POOLING_TYPE_NONE ? batch.n_tokens : s;
            s = 0;
            common_batch_clear(batch);
        }

        // add to batch
        batch_add_seq(batch, input, s);
        s += 1;
    }
    // final batch
    float * out = emb + e * n_embd;
    batch_decode(context, batch, out, s, n_embd, 2);

    {
        const int32_t embd_normalize = 2;
        printf("\n");

        if (llama_pooling_type(context) == LLAMA_POOLING_TYPE_NONE) {
            for (int j = 0; j < n_embd_count; j++) {
                printf("embedding %d: ", j);
                for (int i = 0; i < std::min(3, n_embd); i++) {
                    if (embd_normalize == 0) {
                        printf("%6.0f ", emb[j * n_embd + i]);
                    } else {
                        printf("%9.6f ", emb[j * n_embd + i]);
                    }
                }
                printf(" ... ");
                for (int i = n_embd - 3; i < n_embd; i++) {
                    if (embd_normalize == 0) {
                        printf("%6.0f ", emb[j * n_embd + i]);
                    } else {
                        printf("%9.6f ", emb[j * n_embd + i]);
                    }
                }
                printf("\n");
            }
        } else if (llama_pooling_type(context) == LLAMA_POOLING_TYPE_RANK) {
            for (int j = 0; j < n_embd_count; j++) {
                // NOTE: if you change this log - update the tests in ci/run.sh
                printf("rerank score %d: %8.3f\n", j, emb[j * n_embd]);
            }
        } else {
            // print the first part of the embeddings or for a single prompt, the full embedding
            for (size_t j = 0; j < prompts.size(); j++) {
                printf("embedding %d: ", j);
                for (size_t i = 0; i < (prompts.size() > 1 ? std::min(16, n_embd) : n_embd); i++) {
                    if (embd_normalize == 0) {
                        printf("%6.0f ", emb[j * n_embd + i]);
                    } else {
                        printf("%9.6f ", emb[j * n_embd + i]);
                    }
                }
                printf("\n");
            }

            // print cosine similarity matrix
            if (prompts.size() > 1) {
                printf("\n");
                printf("cosine similarity matrix:\n\n");
                for (int i = 0; i < prompts.size(); i++) {
                    printf("%6.6s ", prompts[i].c_str());
                }
                printf("\n");
                for (size_t i = 0; i < prompts.size(); i++) {
                    for (size_t j = 0; j < prompts.size(); j++) {
                        float sim = common_embd_similarity_cos(emb + i * n_embd, emb + j * n_embd, n_embd);
                        printf("%6.2f ", sim);
                    }
                    printf("%1.10s", prompts[i].c_str());
                    printf("\n");
                }
            }
        }
    }
    llama_perf_context_print(context);

    // clean up
    llama_batch_free(batch);
    llama_free_model(model);
    model = nullptr;
    llama_free(context);
    context = nullptr;
    llama_backend_free();
    return 0;
}
