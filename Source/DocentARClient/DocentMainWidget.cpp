#include "DocentMainWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "DocentNetworkActor.h"

// 안드로이드 JNI 연동 헤더
#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

// 스태틱 인스턴스 초기화
TWeakObjectPtr<UDocentMainWidget> UDocentMainWidget::Instance = nullptr;

void UDocentMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// JNI 통신용 스태틱 인스턴스 저장
	Instance = this;

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

	// 안드로이드 환경 한정 갤러리 액티비티 호출
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jmethodID OpenGalleryMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "OpenAndroidGallery", "()V", false);
		if (OpenGalleryMethod)
		{
			FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, OpenGalleryMethod);
		}
	}
#endif
}

void UDocentMainWidget::OnClickedSendImage()
{
	// 캐싱된 데이터 및 네트워크 액터 유효성 검증
	if (!NetworkActor || CachedImageData.Num() == 0) return;

	if (TextStatus) TextStatus->SetText(FText::FromString(TEXT("상태: 액자로 사진 전송 중...")));

	// 캐싱된 이미지 데이터 서버 송신
	NetworkActor->SendImageData(CachedImageData);
}

void UDocentMainWidget::OnImageSelected(const FString& ImagePath)
{
	if (TextStatus) TextStatus->SetText(FText::FromString(TEXT("상태: 이미지 바이트 변환 중...")));

	TArray<uint8> ImageData;

#if PLATFORM_ANDROID
	// JNI 환경 포인터 획득
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		// FString 문자열 JNI 규격 변환
		jstring jUriString = Env->NewStringUTF(TCHAR_TO_UTF8(*ImagePath));

		// Java GetImageData 함수 메모리 탐색
		jmethodID GetImageDataMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetImageData", "(Ljava/lang/String;)[B", false);

		if (GetImageDataMethod)
		{
			// Java 함수 호출 및 바이트 배열 수신
			jbyteArray jByteArray = (jbyteArray)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, GetImageDataMethod, jUriString);

			if (jByteArray)
			{
				// 수신 배열 크기 측정 및 C++ TArray 메모리 할당
				int32 ArraySize = Env->GetArrayLength(jByteArray);
				ImageData.SetNumUninitialized(ArraySize);

				// JNI 메모리 블록 C++ 복사
				Env->GetByteArrayRegion(jByteArray, 0, ArraySize, reinterpret_cast<jbyte*>(ImageData.GetData()));

				// JNI 로컬 참조 해제
				Env->DeleteLocalRef(jByteArray);
			}
		}
		// JNI 로컬 참조 해제
		Env->DeleteLocalRef(jUriString);
	}
#endif

	// 변환된 데이터 클래스 멤버 변수에 캐싱
	CachedImageData = ImageData;

	if (TextStatus) TextStatus->SetText(FText::FromString(TEXT("상태: 사진 준비 완료! 전송 버튼을 눌러주세요.")));
}

// Java에서 호출할 C++ JNI 네이티브 함수 구현
#if PLATFORM_ANDROID
extern "C" JNIEXPORT void JNICALL Java_com_epicgames_unreal_GameActivity_nativeOnImageSelected(JNIEnv* jenv, jobject thiz, jstring imagePath)
{
	const char* chars = jenv->GetStringUTFChars(imagePath, 0);
	FString Path = FString(UTF8_TO_TCHAR(chars));
	jenv->ReleaseStringUTFChars(imagePath, chars);

	// 게임 스레드 안전성 확보 후 위젯 콜백 실행
	AsyncTask(ENamedThreads::GameThread, [Path]()
		{
			if (UDocentMainWidget::Instance.IsValid())
			{
				UDocentMainWidget::Instance->OnImageSelected(Path);
			}
		});
}
#endif