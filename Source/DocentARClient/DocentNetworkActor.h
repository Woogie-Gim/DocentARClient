#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "DocentNetworkActor.generated.h"

struct FARGyroPacket
{
	float Qx = 0.0f;
	float Qy = 0.0f;
	float Qz = 0.0f;
	float Qw = 1.0f;
};

UCLASS()
class DOCENTARCLIENT_API ADocentNetworkActor : public AActor
{
	GENERATED_BODY()

public:
	ADocentNetworkActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

private:
	FSocket* ClientSocket = nullptr;
	bool bIsConnected = false;

	void SendGyroData(float Qx, float Qy, float Qz, float Qw);

public:
	UPROPERTY(EditAnywhere, Category = "Network")
	FString ServerIP = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, Category = "Network")
	int32 ServerPort = 9000;

	bool ConnectToServer();
};