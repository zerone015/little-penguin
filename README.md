# 🐧 Little Penguin

이 프로젝트는 리눅스 커널 프로그래밍을 단계적으로 학습하기 위한 과제 시리즈입니다.  
각 단계별로 주어진 과제를 수행하며, 커널의 구성요소를 하나씩 구현하고 분석해 나갑니다.   
최종적으로는 리눅스 커널의 서브시스템을 유지보수할 수 있는 기반을 다지는 것을 목표로 합니다.

## ex00
이 단계의 목표는 Linus Torvalds의 Git 저장소에서 최신 리눅스 커널 소스를 다운로드하고,  
설정·빌드·부팅까지 완료하는 것이다.   

#### 요구 사항 
Linus의 Git 저장소에서 최신 커널 소스를 클론하고, CONFIG_LOCALVERSION_AUTO=y 옵션을 활성화한다.   

#### 제출물
- 커널 부팅 로그 파일
- .config 파일

#### 구현 내용
먼저 Linus Torvalds의 공식 커널 저장소에서 최신 소스를 클론해야 한다.      
다음 명령을 사용하여 최신 개발 트리를 받을 수 있다.   
```bash
git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
```
필수 선행 과제인 ft_linux를 통해 구축한 커스텀 배포판에 커널을 빌드해야 하므로,   
기존 설정을 그대로 따르는 편이 좋다.     

추가로 과제 요구사항에 따라 CONFIG_LOCALVERSION_AUTO=y 옵션을 활성화해야 한다.   
이 옵션은 커널을 빌드할 때 버전 문자열 끝에 Git 커밋 해시를 자동으로 추가하여,   
현재 빌드가 정확히 어떤 커밋을 기준으로 생성되었는지를 구분할 수 있게 해준다.    

설정이 올바르게 적용되었다면 부팅 후 uname -r 명령을 실행했을 때   
다음과 같이 커밋 해시가 포함된 버전이 표시된다.
```bash
6.18.0-rc3-00016-ge53642b87a4f
```
   
부팅이 성공하면 dmesg 명령을 사용해 커널 초기화 로그를 확인할 수 있으며,   
이를 파일로 저장해 제출하면 된다.      

#### 참고 자료
- [Installation of the kernel](https://www.linuxfromscratch.org/lfs/view/stable/chapter10/kernel.html)

## ex01
이 단계의 목표는 “Hello World” 커널 모듈을 작성하고,   
리눅스 커널이 모듈을 로드하고 해제하는 과정을 이해하는 것이다.   

#### 요구 사항
다음 동작을 수행하는 간단한 “Hello World” 커널 모듈을 작성해야 한다.   
```c
sudo insmod main.ko  
dmesg | tail -1  
# [Wed May 13 12:59:18 2015] Hello world!

sudo rmmod main.ko  
dmesg | tail -1  
# [Wed May 13 12:59:24 2015] Cleaning up module.
```
모든 시스템에서 컴파일 가능해야 하며, 커널 버전에 따라 동작이 달라지지 않도록 주의해야 한다.    

#### 제출물
- Makefile
- source code

#### 구현 내용
커널 모듈은 실행 중인 커널에 동적으로 로드할 수 있는 드라이버이다.  
내장 방식만 사용하면 하드웨어 추가나 드라이버 수정 때마다 커널을 다시 빌드해야 하며,  
핫플러깅 환경에서는 이 방식이 사실상 불가능하다.  

모듈을 사용하면 insmod, rmmod 명령으로 실행 중인 커널에  
드라이버를 쉽게 추가하거나 제거할 수 있다.

이번 단계에서는 printk()로 메시지를 출력하는 간단한 모듈을 작성한다.   
이는 사용자 공간의 printf()와 유사하지만, 메시지가 커널 링버퍼에 기록된다는 점이 다르다.   

커널 링버퍼는 커널 로그 메시지를 저장하는 순환 버퍼로,   
부팅 과정 및 커널 동작 중 발생하는 로그를 보관하며 dmesg를 통해 확인할 수 있다.   

pr_info(), pr_warn() 등의 매크로도 존재하지만 비교적 최근에 도입된 것이므로   
버전 호환성을 위해 printk()를 사용했다.   

#### init_module()과 module_init()  
초기 리눅스 커널 모듈은 init_module()과 cleanup_module() 함수를 정의하는 방식이었다.   
하지만 최신 커널에서는 이 방식이 허용되지 않으며, objtool에서 컴파일 오류가 발생한다.   
따라서 반드시 module_init()과 module_exit() 매크로를 사용해야 한다.   

커널 드라이버는 모듈 형태뿐 아니라 내장 방식으로도 빌드될 수 있으므로   
양쪽 환경에서 모두 문제없이 작동하도록 작성해야 한다. 

기존 init_module() 방식은 모듈 기반을 전제로 하여,   
내장 드라이버에서는 이름 충돌 때문에 사용할 수 없다.   
여러 드라이버가 존재할 때 모두 init_module이라는 심볼을 갖게 되기 때문이다.   

반면 module_init() 매크로는 다음과 같이 동작한다.   
- 내장 드라이버(obj-y): initcall 테이블에 초기화 함수 등록 → 부팅 시 커널이 호출
- 모듈(obj-m): init_module 심볼로 자동 aliasing → 기존 방식과 동일하게 처리

#### initcall 테이블
커널은 부팅 시 수많은 드라이버를 초기화해야 한다.   
만약 커널 코드 내에서 직접 초기화 함수를 호출한다면 다음과 같은 문제가 발생한다.
- 함수명 충돌 위험
- 드라이버가 추가될 때마다 커널 코드를 수정해야 함

이를 해결하기 위해 initcall 테이블이 존재한다.   

드라이버는 자신의 초기화 함수를 initcall 테이블에 등록하고,   
커널은 부팅 단계에서 해당 테이블을 순회하며 초기화 함수를 호출한다.    

이 방식은 가상 주소를 저장하므로 심볼 이름 충돌 문제를 제거하고,   
커널이 개별 드라이버를 직접 알 필요가 없어 결합도를 낮춘다.   
즉, 확장성과 유지보수성이 높은 구조다.   

#### __init, __exit, __initdata, __exitdata
초기화 함수 및 초기화 전용 데이터는 부팅 후 다시 사용되지 않는다.   
내장 드라이버는 언로드되지 않으므로 cleanup 코드 및 데이터는 필요하지 않다.   

리눅스 커널은 이를 별도 섹션(.init.text, .init.data, .exit.text, .exit.data)에 배치하고   
초기화가 끝나면 해당 메모리를 해제하여 재사용한다. 즉, 메모리 최적화를 위한 구조다.  

문서(Linux Kernel Module Programming Guide)에서는 이 최적화가 내장 드라이버에서만 유효하다고 설명하지만,    
실제 소스 트리를 확인해 본 결과 커널 모듈에서도 로드 완료 후 init 섹션이 해제되는 최적화가 수행되고 있었다.   
다만, 모듈은 언로드될 수 있으므로 exit 섹션은 해제되지 않는다.   
이러한 차이는 해당 문서가 오래된 커널 버전을 기준으로 작성되었기 때문으로 보인다.

#### 참고 자료
- [The Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
 
 ## ex02
이 단계의 목표는 커널 버전 문자열이 구성되는 방식을 이해하고,   
Linux 커널에 패치를 제출하는 절차를 익히는 것이다.

#### 요구 사항
커널 소스 트리 최상위 Makefile에서 EXTRAVERSION 값에 -thor_kernel를 추가한 뒤,   
커널을 재빌드하고 재부팅하여 uname -r 출력에 해당 문자열이 포함되는지 확인해야 한다.   

#### 제출물
- 커널 부팅 로그
- Makefile 수정 패치 파일   
  (Linux 제출 표준 준수: Documentation/SubmittingPatches 참고)

#### 구현 내용
이미 커널 소스와 구성 환경이 준비되어 있다면, 이번 과제는 Makefile의 단 한 줄만 수정하면 된다.   
Linux 커널 최상단 Makefile에는 다음과 같은 버전 정의가 포함된다.   
```Makefile
VERSION = 6
PATCHLEVEL = 8
SUBLEVEL = 0
EXTRAVERSION =
```
여기서 EXTRAVERSION 값을 다음과 같이 변경한다.   
```Makefile
EXTRAVERSION = -thor_kernel
```
EXTRAVERSION은 리눅스 업스트림 릴리즈 관리(-rc1, -rc2 등)에 사용되는 전통적인 필드이다.  

패치 파일은 다음 명령으로 생성한다.   
```bash
git add Makefile
git commit -s -m "Add thor_kernel tag to EXTRAVERSION"
git format-patch -1
```

## ex03
이 단계의 목표는 리눅스 커널 코딩 스타일을 숙지하고, 코드에 적용해보는 것이다.

#### 요구 사항
아래의 주어진 C 파일을 리눅스 코딩 규칙에 맞게 수정해야 한다.
```c
#include
#include
#include
#include

int do_work(int *my_int, int retval) {
    int x;
    int y = *my_int;
    int z;

    for (x = 0; x < my_int; ++x)
    {
        udelay(10);
    }

    if (y < 10)
        /* That was a long sleep, tell userspace about it */
        pr_info("We slept a long time!");

    z = x * y;
    return z;

    return 1;
}

int my_init(void)
{
    int x = 10;
    x = do_work(&x, x);
    return x;
}

void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);
```

#### 제출물
- 수정된 C 파일

#### 구현 내용
리눅스 코딩 스타일 문서를 참고하여 코드 형식을 수정하면 된다.   
수정 후에는 리눅스 소스 트리에서 아래 명령으로 스타일 위반 여부를 확인할 수 있다.
```bash
./scripts/checkpatch.pl --no-tree --file <source file>
```

## ex04
이 단계의 목표는 USB 키보드가 연결될 때 커널 모듈이 자동으로 로드되도록 구성하고,   
udev 기반의 동적 장치 관리 방식을 이해하는 것이다.

#### 요구 사항
ex01에서 작성한 커널 모듈을 수정하여 USB 키보드 연결 시 자동으로 로드되도록 설정해야 한다.   

#### 제출물
- 소스 코드
- udev 규칙 파일

#### 구현 내용
ex01에서는 insmod 명령을 사용해 모듈을 직접 로드했다.   
이번 단계에서는 USB 키보드가 연결되면 모듈이 자동으로 로드되도록 해야 한다.   
이를 위해 udev 규칙 파일을 작성하고 시스템의 hotplug 이벤트 처리 흐름을 이해해야 한다.   

#### udev
udev는 커널이 감지한 장치들에 대해 사용자 공간 정책을 적용하여   
/dev 디렉터리를 관리하는 동적 장치 관리자이다.

리눅스에서는 /dev/* 장치 파일을 통해 장치에 접근한다.  
초기에는 핫플러깅 개념이 없었기 때문에, 가능한 장치 파일들을 /dev 아래에  
정적으로 미리 생성해 두는 방식이 사용되었다.

그러나 USB와 같은 핫플러그 장치가 등장하면서 장치 파일을 동적으로 관리해야 할 필요가 생겼고,  
이를 위해 devfs가 도입되었다.  
하지만 devfs는 장치 이름을 커널이 직접 결정했기 때문에  
사용자 공간에서 장치 명명 규칙을 제어할 수 없었고,  
레이스 컨디션 문제도 있어 결국 커널에서 제거되었다.

현재는 devtmpfs + sysfs + udev 방식이 사용된다.  
핵심 흐름은 다음과 같다.   

1. 부팅 시 udevd가 실행된다.
2. udevd는 udev 규칙 파일을 읽어 메모리에 로드한다.
3. 커널이 새로운 장치를 감지하면, 장치 정보를 sysfs(/sys)에 등록한다.
4. 커널은 uevent를 발생시키고, netlink 소켓을 통해 udevd에 전달한다.
5. udevd는 uevent를 수신한 뒤, sysfs에서 해당 장치의 속성을 조회한다.
6. udevd는 장치 속성을 기반으로 규칙을 매칭하고, 매칭된 규칙에 따라 드라이버 로딩을 처리한다.
7. 드라이버가 장치를 등록하면, 커널의 devtmpfs가 /dev 아래에 기본 장치 파일을 생성한다.
8. udevd는 추가로 사용자 공간 정책을 적용한다:
   - 장치 파일 이름
   - 접근 권한 및 소유자 설정  
   - 심볼릭 링크 생성  
   - 사용자 정의 스크립트 실행

udev 규칙은 파일명 기준으로 사전순으로 적용되며,   
디렉터리 우선순위는 다음과 같다.

1. /run/udev/rules.d/   (런타임 임시 규칙)
2. /etc/udev/rules.d/   (사용자 정의 규칙)
3. /usr/lib/udev/rules.d/ (배포판 기본 규칙)

자세한 내용은 man udev에서 확인할 수 있다.

#### coldplugging
udevd가 실행되기 전에 커널이 생성한 장치 파일들은  
규칙 파일에 정의된 사용자 공간 정책이 적용되지 않은 상태이다.

udevd가 시작되면 /sys 아래의 기존 장치들을 스캔하고,  
각 장치에 대해 uevent를 재처리하여 규칙을 적용한다.  
이 과정을 coldplugging이라고 한다.

#### insmod vs modprobe
insmod는 지정한 .ko 파일을 직접 커널에 적재한다.   
파일 경로를 요구하며, 의존 모듈을 자동으로 로드하지 않는다.

반면 modprobe는 모듈 alias를 기반으로 적재하며,   
필요한 의존 모듈도 함께 로드한다.

커널은 장치를 감지하면 해당 장치 정보를 기반으로 MODALIAS 문자열을 생성하여 uevent에 포함한다.   
기본 udev 규칙이 적용되는 경우, udev는 이를 받아 다음과 같이 모듈을 로드한다.
```bash
modprobe $MODALIAS
```
이는 기본 규칙 파일인 /usr/lib/udev/rules.d/80-drivers.rules에서 확인할 수 있다.

modprobe 기반 자동 로드를 사용하려면 드라이버는
```c
MODULE_DEVICE_TABLE(...)
```
을 통해 장치 정보를 내보내야 하며,   
depmod -a 이후 /lib/modules/.../modules.alias에 alias가 반영된다.

참고로, 최신 systemd-udev 환경에서는 기본 규칙 파일을 보면   
외부 modprobe 호출 대신 builtin kmod라는 udev 내장 모듈 로더가 사용되는 규칙이 포함되어 있다.

#### 참고 자료
- [Hotplugging with udev](https://bootlin.com/doc/legacy/udev/udev.pdf)
- [Overview of Device and Module Handling](https://www.linuxfromscratch.org/lfs/view/stable/chapter09/udev.html)
- [Linux Device Drivers, 3rd Edition, Chapter 14](https://lwn.net/Kernel/LDD3/)

## ex05
이번 과제의 목표는 misc 디바이스 드라이버를 작성하고,   
드라이버가 사용자 공간과 데이터를 주고받는 방식을 이해하는 것이다.

#### 요구 사항
ex01에서 작성한 커널 모듈을 수정하여 misc 디바이스 드라이버로 만든다.
- 동적 마이너 번호를 사용해야 한다.
- 디바이스 노드 경로는 /dev/fortytwo여야 한다.
- read()는 사용자에게 학생 로그인 문자열을 반환한다.
- write()는 입력값이 로그인 문자열과 일치하는지 확인하여,   
  일치하면 정상 반환, 일치하지 않으면 "invalid value" 오류를 반환해야 한다.
- 모듈이 로드될 때 장치를 등록하고, 언로드 시 해제해야 한다.

#### 제출물
- 소스 코드

#### 구현 내용
리눅스 커널의 장치 드라이버는 하드웨어 장치를 제어하기 위한 인터페이스로,   
데이터 처리 방식에 따라 세 가지로 나뉜다.

| 구분 | 데이터 단위 | 접근 방식 | 대표 장치 | 비고 |
|------|--------------|------------|-------------|------|
| 문자 드라이버 | 바이트 단위 | 순차 접근 | 키보드, 마우스, 콘솔 등 | /dev/tty, /dev/null 등 |
| 블록 드라이버 | 블록 단위 | 임의 접근 | 하드디스크, SSD 등 | /dev/sda, /dev/loop0 등 |
| 네트워크 드라이버 | 프레임 단위 | 패킷 기반 | NIC, 무선랜 카드 등 | eth0, wlan0 등 |

Misc 드라이버는 문자 디바이스의 일종이지만,   
메이저 번호가 항상 10번으로 고정되어 있고 등록 절차가 간단하다는 점이 특징이다.   
테스트용이나 단순한 장치를 빠르게 구현할 때 자주 사용된다.

리눅스에서 장치는 메이저 번호와 마이너 번호로 구분된다.   
메이저 번호는 드라이버 자체를 식별하고, 마이너 번호는 해당 드라이버가 관리하는 개별 장치를 구분한다.   

결국 각 장치는 MAJOR:MINOR 형태의 고유한 번호로 식별된다.

#### misc_register()와 misc_deregister()
드라이버 초기화 함수에서 misc_register(struct miscdevice *)를 호출하면   
커널이 /dev 디렉터리에 장치 파일을 자동으로 생성한다.   
언로드 시에는 misc_deregister(struct miscdevice *)를 호출하여 장치 파일을 제거한다.   

이 구조체에는 read, write 등 사용자 공간의 시스템 콜과 연결되는   
함수 포인터가 포함되어 있다.   
즉, 사용자가 /dev/fortytwo를 open(), read(), write()하면   
해당 함수 포인터에 등록된 드라이버 코드가 실행된다.

#### copy_to_user()와 copy_from_user()
드라이버에서 사용자 공간 포인터에 직접 접근하는 것은 금지되어 있다.   
사용자 프로그램이 잘못된 주소를 전달하면 유저 모드에서는 해당 프로세스만 종료되지만,    
커널 모드에서 잘못된 주소를 참조하면 커널 패닉이 발생해 시스템 전체가 중단된다.    
이를 방지하기 위해 커널은 copy_to_user()와 copy_from_user()라는 전용 함수를 제공한다.    

이 함수들은 단순히 메모리를 복사하는 것이 아니라,    
예외 처리를 통해 커널이 안전하게 유저 공간 메모리에 접근할 수 있도록 보장한다.    
내부적으로는 예외 테이블을 사용하여 페이지 폴트가 발생해도 커널이 패닉을 일으키지 않도록 처리하며,    
이를 통해 일부만 복사되는 상황도 정상적으로 처리된다.

또한 이 함수들은 전달된 주소와 범위가 실제로 유저 공간에 속하는지도 검증한다.   
사용자가 악의적으로 커널 공간 주소를 유저 버퍼인 척 전달할 수도 있기 때문이다.    
만약 이런 검증이 없다면, 사용자는 커널 메모리를 덮어써서 시스템을 완전히 장악할 수 있다.    

copy_to_user()와 copy_from_user()는 이 외에도 다양한 보호 장치를 포함하고 있다.   
잘못된 커널 버퍼 주소나 크기에 대한 컴파일타임 경고를 발생시키고, 런타임 시에는 오류를 로깅한다.   
또한 원자적 컨텍스트에서 잘못된 sleep 호출을 감지하고,   
스펙터와 같은 투기적 실행 취약점을 방지하기 위한 메모리 펜스 코드도 포함되어 있다.   
이런 이유로 커널에서 유저 공간에 접근할 때는 반드시 이 함수들을 사용해야 한다.

#### 참고 자료
- [Linux Device Driver Tutorial Part 4 – Character Device Driver](https://embetronicx.com/tutorials/linux/device-drivers/character-device-driver-major-number-and-minor-number/)
- [Misc Device Driver – Linux Device Driver Tutorial Part 32](https://embetronicx.com/tutorials/linux/device-drivers/misc-device-driver/)

## ex06
