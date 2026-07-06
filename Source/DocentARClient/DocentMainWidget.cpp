#include "DocentMainWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "DocentNetworkActor.h"

void UDocentMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 함수 바인딩
	if (BtnConnect) BtnConnect->OnClicked.AddDynamic(this, &UDocentMainWidget::OnClickedConnect);
	if (BtnToggleGyro) BtnToggleGyro->OnClicked.AddDynamic(this, &UDocentMainWidget::OnClickedToggleGyro);
	if (BtnPickImage) BtnPickImage->OnClicked.AddDynamic(this, &UDocentMainWidget::OnClickedPickImage);
	if (BtnSendImage) BtnSendImage->OnClicked.AddDynamic(this, &UDocentMainWidget::OnClickedSendImage);

	// 월드에 배치된 통신 액터 탐색 및 참조 저장
	NetworkActor = Cast<ADocentNetworkActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ADocentNetworkActor::StaticClass()));
}

void UDocentMainWidget::OnClickedConnect()
{
	if (!NetworkActor || !InputServerIP) return;

	// 텍스트 박스의 입력 IP 추출
	FString TargetIP = InputServerIP->GetText().ToString();
	if (TargetIP.IsEmpty()) TargetIP = TEXT("127.0.0.1");

	// 액터 IP 교체 및 서버 재연결 시도
	NetworkActor->ServerIP = TargetIP;
	bool bConnected = NetworkActor->ConnectToServer();

	// 연결 결과에 따른 상태 텍스트 갱신
	if (TextStatus)
	{
		TextStatus->SetText(bConnected ? FText::FromString(TEXT("상태: DX12 연결 성공!")) : FText::FromString(TEXT("상태: 연결 실패")));
	}
}

void UDocentMainWidget::OnClickedToggleGyro()
{
	bIsGyroSending = !bIsGyroSending;

	// 액터 자이로 전송 활성화 여부 제어
	if (NetworkActor)
	{
		NetworkActor->SetActorTickEnabled(bIsGyroSending);
	}

	// 토글 상태 텍스트 갱신
	if (TextStatus)
	{
		FString StatusMsg = bIsGyroSending ? TEXT("상태: 자이로 연동 중 [ON]") : TEXT("상태: 자이로 일시정지 [OFF]");
		TextStatus->SetText(FText::FromString(StatusMsg));
	}
}

void UDocentMainWidget::OnClickedPickImage()
{
	// 갤러리 호출 및 사진 선택 로직 진입
	if (TextStatus) TextStatus->SetText(FText::FromString(TEXT("상태: 모바일 갤러리 여는 중...")));
}

void UDocentMainWidget::OnClickedSendImage()
{
	if (!NetworkActor) return;

	// 선택된 사진 바이너리 DX12 서버 송신 로직 진입
	if (TextStatus) TextStatus->SetText(FText::FromString(TEXT("상태: 액자로 사진 전송 중...")));
}