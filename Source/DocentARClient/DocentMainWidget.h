

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DocentMainWidget.generated.h"

// UI 위젯 컴포넌트 전방 선언
class UButton;
class UEditableTextBox;
class UTextBlock;
class UImage;
class ADocentNetworkActor;

UCLASS()
class DOCENTARCLIENT_API UDocentMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// JNI 콜백을 위한 위젯 인스턴스 포인터
	static TWeakObjectPtr<UDocentMainWidget> Instance;

	// 갤러리 이미지 선택 완료 시 호출 콜백
	void OnImageSelected(const FString& ImagePath);

protected:
	// 위젯 초기화 시 버튼 이벤트 바인딩
	virtual void NativeConstruct() override;

protected:
	// IP 입력 텍스트 박스
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InputServerIP;

	// 서버 연결 시도 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* BtnConnect;

	// 현재 연결 및 전송 상태 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextStatus;

	// 자이로 데이터 전송 토글 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* BtnToggleGyro;

	// 스마트폰 사진첩 열기 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* BtnPickImage;

	// 선택된 사진 미리보기 이미지 위젯
	UPROPERTY(meta = (BindWidget))
	UImage* ImagePreview;

	// DX12 액자로 사진 전송 실행 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* BtnSendImage;

private:
	// 연결 버튼 클릭 시 실행
	UFUNCTION()
	void OnClickedConnect();

	// 자이로 토글 버튼 클릭 시 실행
	UFUNCTION()
	void OnClickedToggleGyro();

	// 갤러리 선택 버튼 클릭 시 실행
	UFUNCTION()
	void OnClickedPickImage();

	// 사진 전송 버튼 클릭 시 실행
	UFUNCTION()
	void OnClickedSendImage();

	// 선택된 이미지 바이트 데이터 캐싱
	TArray<uint8> CachedImageData;

private:
	// 월드에 존재하는 네트워크 통신 액터 포인터
	UPROPERTY()
	ADocentNetworkActor* NetworkActor;

	// 자이로 전송 활성화 상태 플래그
	bool bIsGyroSending = true;
};
