#include "UEChatAPI.h"

#include <HttpModule.h>
#include "Interfaces/IHttpResponse.h"
#include <Dom/JsonObject.h>
#include <JsonUtilities.h>

#include "UEChatSettings.h"

UEChatAPI::UEChatAPI()
{
}

UEChatAPI::~UEChatAPI()
{
}

void UEChatAPI::Send(const FString& Message)
{
	const UUEChatSettings* Settings = GetDefault<UUEChatSettings>();
	if(Settings->EndPoint.IsEmpty() || Message.IsEmpty()){
		return;
	}

	FString EndPoint = Settings->EndPoint;
	const FString& APIKey = Settings->APIKey;
	const FString& Model = Settings->Model;

	#if 1
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetField(TEXT("model"), MakeShared<FJsonValueString>(Model));
    {
		TArray<TSharedPtr<FJsonValue>> MessageArray;

		TSharedPtr<FJsonObject> Value;
		Value = MakeShared<FJsonObject>();
		Value->SetField(TEXT("role"), MakeShared<FJsonValueString>(TEXT("system")));
		Value->SetField(TEXT("content"), MakeShared<FJsonValueString>(TEXT("You are a helpful assistant.")));
		MessageArray.Add(MakeShared<FJsonValueObject>(Value));

		Value = MakeShared<FJsonObject>();
		Value->SetField(TEXT("role"), MakeShared<FJsonValueString>(TEXT("user")));
		Value->SetField(TEXT("content"), MakeShared<FJsonValueString>(Message));
		MessageArray.Add(MakeShared<FJsonValueObject>(Value));

		JsonObject->SetArrayField(TEXT("messages"), MessageArray);
	}

	FString RequestText;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestText);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	const auto RequestStringConv = StringCast<UTF8CHAR>(*RequestText);
	const char* const RequestString = reinterpret_cast<const char*>(RequestStringConv.Get());

	TArray<uint8> Data;
	Data.SetNum(RequestStringConv.Length());
	FMemory::Memcpy(&Data[0], RequestString, RequestStringConv.Length());

	if(!EndPoint.EndsWith(TEXT("/"))){
		EndPoint.AppendChar(TEXT('/'));
	}
	EndPoint.Append(TEXT("v1/chat/completions"));

	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(EndPoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("charset"), TEXT("UTF-8"));
	Request->SetHeader(TEXT("Authorization:"), TEXT("Bearer ") + APIKey);
	Request->OnProcessRequestComplete().BindRaw(this, &UEChatAPI::OnComplete);
	Request->SetContent(Data);
	Request->ProcessRequest();
	#endif
}

UEChatAPI::FOnResponse& UEChatAPI::OnResponse()
{
	return OnResponse_;
}

void UEChatAPI::OnComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	FString String;
	if(!bConnectedSuccessfully || nullptr == Response){
		Request->CancelRequest();
		OnResponse_.ExecuteIfBound(false, String);
		return;
	}
	int32 ResponceCode = Response->GetResponseCode();
	if(200 != ResponceCode) {
		Request->CancelRequest();
		OnResponse_.ExecuteIfBound(false, String);
		return;
	}
	const TArray<uint8>& Content = Response->GetContent();
	if(Content.Num()<=0){
		OnResponse_.ExecuteIfBound(false, String);
		return;
	}
	FString ContentStr(Content.Num(), reinterpret_cast<const UTF8CHAR*>(&Content[0]));
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MoveTemp(ContentStr));
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	if(!FJsonSerializer::Deserialize(JsonReader, JsonObject)){
		OnResponse_.ExecuteIfBound(false, String);
		return;
	}
	const TArray<TSharedPtr<FJsonValue>>& Choices = JsonObject->GetArrayField(TEXT("choices"));
	if(Choices.Num()<=0){
		OnResponse_.ExecuteIfBound(false, String);
		return;
	}
	const TSharedPtr<FJsonObject>& Message = Choices[0]->AsObject()->GetObjectField(TEXT("message"));
	FString MessageContent = Message->GetStringField(TEXT("content"));
	OnResponse_.ExecuteIfBound(true, MessageContent);
}

