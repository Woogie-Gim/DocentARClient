
#include "DocentNetworkActor.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Kismet/GameplayStatics.h"

ADocentNetworkActor::ADocentNetworkActor()
{
	// 매 프레임 데이터 송신을 위해 Tick 활성화
	PrimaryActorTick.bCanEverTick = true;
}

void ADocentNetworkActor::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 DX12 서버로 TCP 연결 시도
	bIsConnected = ConnectToServer();
	if (bIsConnected)
	{
		UE_LOG(LogTemp, Log, TEXT("DX12 서버(포트 %d) 연결 성공!"), ServerPort);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DX12 서버 연결 실패. IP 및 포트를 확인하세요."));
	}
}

void ADocentNetworkActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 액터 파괴 시 소켓 안전 해제
	if (ClientSocket != nullptr)
	{
		ClientSocket->Close();
		ISocketSubsystem::Get()->DestroySocket(ClientSocket);
		ClientSocket = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

bool ADocentNetworkActor::ConnectToServer()
{
	// 소켓 서브시스템 인스턴스 획득
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
	if (SocketSubsystem == nullptr) return false;

	// TCP 스트림 소켓 생성
	ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("DocentARClient"), false);
	if (ClientSocket == nullptr) return false;

	// 문자열 IP를 FIPv4Address 구조체로 변환
	FIPv4Address IPAddress;
	FIPv4Address::Parse(ServerIP, IPAddress);

	// 인터넷 주소 객체 생성 및 포트 바인딩
	TSharedRef<FInternetAddr> InternetAddr = SocketSubsystem->CreateInternetAddr();
	InternetAddr->SetIp(IPAddress.Value);
	InternetAddr->SetPort(ServerPort);

	// 서버 연결 요청 (블로킹 통신 시도)
	return ClientSocket->Connect(*InternetAddr);
}

void ADocentNetworkActor::SendGyroData(float Qx, float Qy, float Qz, float Qw)
{
	if (!bIsConnected || ClientSocket == nullptr) return;

	// 전송할 패킷 구조체 조립
	FARGyroPacket Packet;
	Packet.Qx = Qx;
	Packet.Qy = Qy;
	Packet.Qz = Qz;
	Packet.Qw = Qw;

	// 바이트 전송 바인딩 변수
	int32 BytesSent = 0;

	// 메모리 주소를 바이트(uint8*) 포인터로 캐스팅하여 바이너리 전송
	ClientSocket->Send(reinterpret_cast<const uint8*>(&Packet), sizeof(FARGyroPacket), BytesSent);
}

void ADocentNetworkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsConnected)
	{
		// 현재 월드의 첫 번째 플레이어 컨트롤러 획득
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			FVector CamLoc;
			FRotator CamRot;
			// 모바일 AR 기기가 움직일 때 변하는 실제 카메라 시선 회전값 추출
			PC->GetPlayerViewPoint(CamLoc, CamRot);

			// FRotator를 쿼터니언(FQuat)으로 변환
			FQuat GyroQuat = CamRot.Quaternion();

			// DX12 서버로 실시간 자이로 쿼터니언 송신
			SendGyroData(GyroQuat.X, GyroQuat.Y, GyroQuat.Z, GyroQuat.W);
		}
	}
}