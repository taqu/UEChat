#pragma once
/**
 @author t-sakai
 */
#include "CoreMinimal.h"
#include "HttpModule.h"

class UECHAT_API UEChatAPI
{
public:
	using FOnResponse = TTSDelegate<void(bool,const FString&)>;

	UEChatAPI();
	~UEChatAPI();
	void Send(const FString& Message);
	FOnResponse& OnResponse();
private:
	UEChatAPI(const UEChatAPI&) = delete;
	UEChatAPI& operator=(const UEChatAPI&) = delete;

	void OnComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	FOnResponse OnResponse_;
};
