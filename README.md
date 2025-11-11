# 🐧 Little Penguin

이 프로젝트는 리눅스 커널 프로그래밍을 단계적으로 학습하기 위한 과제 시리즈입니다.  
각 단계별로 주어진 과제를 수행하며, 커널의 구성요소를 하나씩 구현하고 분석해 나갑니다.   
최종적으로는 리눅스 커널의 서브시스템을 유지보수할 수 있는 기반을 다지는 것을 목표로 합니다.

## ex00
이 단계의 목표는 최신 리눅스 커널 소스를 다운로드하고, 빌드하고, 부팅하는 것이다.   

#### 요구 사항 
Linus의 Git 저장소에서 최신 커널 소스를 클론하고, CONFIG_LOCALVERSION_AUTO=y 옵션을 활성화한다.   

#### 제출물
- 커널 부팅 로그
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
내장 방식만 사용하면 하드웨어 추가나 드라이버 수정 때마다 커널을 다시 빌드해야 한다.   
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

드라이버는 자신의 초기화 함수를 함수 포인터 형태로 initcall 테이블에 등록하고,   
커널은 부팅 단계에서 해당 테이블을 순회하며 각 함수 포인터를 호출한다.

이 방식은 심볼이 아닌 주소를 사용하므로 심볼 충돌 문제를 방지하고,   
커널이 개별 드라이버를 직접 알 필요가 없어 결합도를 낮춘다.   
즉, 확장성과 유지보수성이 뛰어난 구조다. 

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
이 단계의 목표는 Linux 커널에 패치를 제출하는 절차를 익히는 것이다.

#### 요구 사항
- ex00에서 사용했던 커널 Git 트리를 이용하여 Makefile을 수정해야 한다.
- EXTRAVERSION 필드를 변경하여, 커널을 수정·재빌드·재부팅했을 때   
  실행 중인 커널 버전 문자열에 -thor_kernel 이 포함되도록 해야 한다.

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

최신 systemd-udev 환경에서는 기본 규칙 파일을 보면   
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
장치 드라이버는 하드웨어 장치를 제어하기 위한 인터페이스로, 데이터 처리 방식에 따라 세 가지로 나뉜다.

| 구분 | 데이터 단위 | 접근 방식 | 대표 장치 | 비고 |
|------|--------------|------------|-------------|------|
| 문자 드라이버 | 바이트 단위 | 순차 접근 | 키보드, 마우스, 콘솔 등 | /dev/tty, /dev/null 등 |
| 블록 드라이버 | 블록 단위 | 임의 접근 | 하드디스크, SSD 등 | /dev/sda, /dev/loop0 등 |
| 네트워크 드라이버 | 프레임 단위 | 패킷 기반 | NIC, 무선랜 카드 등 | eth0, wlan0 등 |

misc 드라이버는 문자 디바이스의 일종이지만,   
메이저 번호가 항상 10번으로 고정되어 있고 등록 절차가 간단하다는 점이 특징이다.   
테스트용이나 단순한 장치를 빠르게 구현할 때 자주 사용된다.

또한 misc 드라이버는 동적 마이너 번호를 간단히 할당받을 수 있다는 장점이 있다.   
miscdevice 구조체의 minor 필드에 MISC_DYNAMIC_MINOR를 지정하면 커널이 사용 가능한 번호를 자동으로 배정한다.   
이 매크로는 255로 정의되어 있으며, 직접 번호를 지정할 때는 0~254를 사용할 수 있다.   
동적으로 할당되는 마이너 번호는 256부터 시작하며, 총 1,048,320개가 존재한다.

리눅스에서 장치는 메이저 번호와 마이너 번호로 구분된다.   
메이저 번호는 드라이버 자체를 식별하고, 마이너 번호는 해당 드라이버가 관리하는 개별 장치를 구분한다.   
결국 각 장치는 MAJOR:MINOR 형태의 고유한 번호로 식별된다.

#### misc_register()와 misc_deregister()
드라이버의 초기화 함수에서 misc_register()를 호출하면 /dev 디렉터리에 해당 장치 파일이 생성된다.   
모듈이 언로드될 때는 misc_deregister()를 호출하여 등록된 장치 파일을 제거한다.

miscdevice 구조체는 file_operations 구조체를 포함하고 있으며,   
이 file_operations 구조체에는 read, write 등 사용자 공간의 시스템 콜과 연결되는 함수 포인터들이 정의되어 있다.   
따라서 사용자가 /dev/fortytwo를 open(), read(), write() 등의 시스템 콜로 접근하면,   
해당 함수 포인터에 등록된 드라이버 코드가 실행된다.

#### copy_to_user()와 copy_from_user()
드라이버에서 사용자 공간 포인터에 직접 접근하는 것은 금지되어 있다.   
유저 모드에서의 잘못된 주소 참조는 해당 유저 프로세스만 종료되지만,   
커널 모드에서 유저가 전달한 잘못된 주소를 참조하면 커널 패닉이 발생해 시스템 전체가 중단된다.   
이를 방지하기 위해 커널은 copy_to_user()와 copy_from_user()라는 전용 함수를 제공한다.    

이 함수들은 단순히 메모리를 복사하는 것이 아니라,    
예외 처리를 통해 커널이 안전하게 유저 공간 메모리에 접근할 수 있도록 보장한다.    
내부적으로는 예외 테이블을 사용하여 의도되지 않은 페이지 폴트가 발생해도   
사전에 등록된 오류 처리 주소로 점프하여 커널이 패닉을 일으키지 않도록 처리하며,    
이를 통해 일부만 복사되는 상황도 정상적으로 처리된다.

또한 이 함수들은 전달된 주소와 범위가 실제로 유저 공간에 속하는지도 검증한다.   
사용자가 악의적으로 커널 공간 주소를 유저 버퍼인 척 전달할 수도 있기 때문이다.    
만약 이런 검증이 없다면, 사용자는 커널 공간을 조작하여 시스템에 대한   
임의의 제어 권한을 얻거나 심각한 보안 취약점을 발생시킬 수 있다.

copy_to_user()와 copy_from_user()는 이 외에도 다양한 보호 장치를 포함하고 있다.   
잘못된 커널 버퍼 주소나 크기에 대한 컴파일타임 오류를 발생시키고, 런타임 시에는 오류를 로깅한다.   
또한 원자적 컨텍스트에서 잘못된 sleep 호출을 감지하고,   
스펙터와 같은 투기적 실행 취약점을 방지하기 위한 메모리 펜스 코드도 포함되어 있다.   
이런 이유로 커널에서 유저 공간에 접근할 때는 반드시 이 함수들을 사용해야 한다.

#### read() 구현
read() 함수는 사용자에게 학생 로그인 문자열을 반환해야 한다.   
이를 직접 copy_to_user()로 구현할 수도 있지만, 커널에는 이미 이 작업을 단순화한    
simple_read_from_buffer()라는 헬퍼 함수가 존재한다.  

이 함수는 내부적으로 copy_to_user()를 호출하고,   
파일 오프셋을 자동으로 관리해주기 때문에 직접 구현하는 것보다 편리하다.   
따라서 본 과제에서는 simple_read_from_buffer()를 사용하여   
유저 공간으로 로그인 문자열을 전달하도록 구현하였다.

#### write() 구현
write()는 사용자로부터 입력받은 문자열이 로그인 문자열과 일치하는지 검사한다.   
커널에는 simple_write_to_buffer()라는 유사한 함수가 존재하지만,   
write()의 경우에는 read()처럼 파일 오프셋을 관리할 필요가 없고   
단순히 입력값이 로그인과 일치하는지만 확인하면 되므로 직접 구현하였다.

입력된 값이 로그인 문자열과 다를 경우, -EINVAL을 반환하도록 하였다.   
이 반환값은 사용자 공간의 glibc syscall wrapper가 받아서   
errno에 EINVAL로 저장된다.   
즉, 유저 레벨에서 write()가 실패하면 echo 등 쉘 명령이   
“Invalid argument”라는 메시지를 출력하게 된다.

#### 테스트 방법
모듈을 정상적으로 빌드하고 로드한 뒤 다음 명령으로 테스트할 수 있다.   
- read 테스트

  ```bash
  cat /dev/fortytwo
  ```
  → 자신의 로그인 문자열이 출력되어야 한다.
- write 테스트

  ```bash
  echo -n "<login>" > /dev/fortytwo
  ```
  → 입력이 일치하면 정상 종료,
    입력이 다르면 echo가 “Invalid argument”를 출력한다.
  
#### 참고 자료
- [Linux Device Driver Tutorial Part 4 – Character Device Driver](https://embetronicx.com/tutorials/linux/device-drivers/character-device-driver-major-number-and-minor-number/)
- [Misc Device Driver – Linux Device Driver Tutorial Part 32](https://embetronicx.com/tutorials/linux/device-drivers/misc-device-driver/)

## ex06
이 단계의 목표는 리눅스 커널의 개발 주기와 기여 과정을 이해하고,   
커널에 직접 기여하기 위해 필요한 절차와 협업 방식을 학습하는 것이다.

#### 요구 사항
- 최신 linux-next 커널을 다운로드하여 빌드 및 부팅한다.
- Documentation/process/의 문서를 읽는다.

#### 제출물
- 커널 부팅 로그

#### 구현 내용
linux-next는 메인라인에 새로운 패치들이 적용되기 전에,   
미리 통합하여 문제를 점검하기 위한 테스트용 통합 트리이다.   
메인라인 커널에 병합되기 전,   
서브시스템 간 충돌이나 회귀 문제를 조기에 발견하고 수정할 수 있도록 한다.

이후 새로운 패치 사항들은 이 linux-next 트리에서   
Linus Torvalds의 승인을 받아 메인라인에 병합되며,   
이 시점부터 릴리즈 후보(rc) 버전으로 개발 주기가 시작된다.

linux-next와 커널의 릴리즈 주기 전반은   
Documentation/process/ 디렉터리에 자세히 설명되어 있다.   
이 문서에는 리눅스 커널이 어떻게 개발되고 릴리즈되는지,   
그리고 어떤 구조로 협업이 이루어지는지가 담겨 있다.   
또한 커널에 기여하고자 하는 초심자가 무엇을 조심해야 하는지,   
어떻게 시작해야 하는지에 대한 실질적인 가이드가 포함되어 있다.   
더불어 드라이버 개발이나 커널 전체 구조를 학습하기 위한 참고 서적과 자료까지 안내되어 있어,   
리눅스 커널 개발을 목표로 하는 사람에게 사실상 필수적인 문서라 할 수 있다.

## ex07
이 단계의 목표는 debugfs를 사용해보고,   
커널 내부 데이터를 사용자 공간에 안전하게 노출하는 방법을 학습하는 것이다.

#### 요구 사항
ex01에서 작성한 모듈을 기반으로 다음과 같은 debugfs 엔트리를 생성해야 한다.
- /sys/kernel/debug/fortytwo/ 디렉터리 생성
- 해당 디렉터리 내에 다음 3개의 파일 생성
  - id: ex05의 /dev/fortytwo와 동일하게 동작해야 함 (모든 사용자, 읽기/쓰기 가능)
  - jiffies: 읽을 때 현재 커널의 jiffies 타이머 값을 반환. (모든 사용자, 읽기 전용)
  - foo: 쓰인 데이터(최대 한 페이지)를 저장하고, 읽으면 그 데이터를 반환해야 함.   
          동시에 읽기/쓰기를 처리하기 위해 적절한 락을 구현해야 함. (root만 쓰기 가능, 모든 사용자 읽기 가능)    
- 모듈 언로드 시, 생성된 모든 debugfs 파일 및 디렉터리를 정리하고,
  할당된 메모리를 해제해야 한다.

#### 제출물
- 소스 코드

#### 구현 내용
debugfs는 디버깅을 위해 설계된 특수 목적의 파일시스템이다.   
/sys/kernel/debug에 마운트되며, CONFIG_DEBUG_FS 옵션이 활성화되어 있어야 사용할 수 있다.

sysfs가 커널 내부 상태를 체계적으로 노출하는 반면,   
debugfs는 “규칙이 없다(The only rule is: there are no rules)”라는 철학에 따라   
개발자가 자유롭게 커널 데이터를 노출하고 테스트 할 수 있다.   
이 때문에 debugfs의 정보들은 커널 버전 간 호환이 보장되지 않으며,   
주로 개발 및 디버깅용 인터페이스로만 사용된다.

디렉터리와 파일은 debugfs_create_dir()과 debugfs_create_file()로 생성하며,   
각 함수는 생성된 dentry 포인터를 반환한다.   
오류 발생 시 에러 포인터를 반환하므로, 커널에서는   
IS_ERR / PTR_ERR / ERR_PTR 매크로를 사용해 에러를 확인하고 처리한다.   
이는 가상 주소 공간의 가장 높은 영역 일부를 에러 포인터 전용으로 예약하여,   
함수 실패 시 에러 코드를 포인터처럼 반환할 수 있게 한다.

모듈 제거 시에는 debugfs_remove()를 호출해   
디렉터리와 그 하위의 모든 엔트리를 재귀적으로 제거해야 한다.

#### id 파일
ex05의 /dev/fortytwo 구현을 그대로 재사용하였다.   
읽으면 학생 로그인 문자열을 반환하고, 쓰면 일치 여부를 확인한다.

#### jiffies 파일
jiffies는 부팅 이후 커널이 발생시킨 타이머 틱의 누적 횟수를 저장하는 전역 변수이다.   
타이머 인터럽트는 HZ 매크로 값(초당 인터럽트 발생 횟수)에 따라 주기적으로 발생하며,   
jiffies 값은 부팅 이후의 경과 시간을 표현하거나 타임아웃 계산 등에 자주 사용된다.   
이를 HZ 값으로 나누면 대략 부팅 후 몇 초가 지났는지 알 수 있고,   
예를 들어 jiffies + HZ * 5처럼 계산하면 5초 후의 시점을 나타낼 수 있다.

jiffies는 x86에서는 unsigned long, x86-64에서는 u64 타입으로 정의되어 있다.   
따라서 32비트 환경에서는 약 49일 후 오버플로가 발생할 수 있으며,   
이를 안전하게 처리하려면 get_jiffies_64() 함수를 사용해야 한다.   
이 함수는 32비트 환경에서도 jiffies 값을 64비트 정수로 안전하게 읽을 수 있도록 구현되어 있다.

자세한 내용은 아래 링크에 정리되어 있다:   
[Jiffies: The Heartbeat of the Linux Operating System](https://blogs.oracle.com/linux/jiffies-the-heartbeat-of-the-linux-operating-system)

#### foo 파일
사용자가 쓴 데이터를 다음 읽기 요청에서 다시 반환하려면   
전역 범위의 내부 버퍼가 필요하며,   
읽기와 쓰기가 동시에 발생할 수 있으므로 이에 대한 동기화가 필요하다.

이 파일은 root만 쓸 수 있고, 일반 사용자들은 읽기만 가능하다.   
읽기 작업이 쓰기보다 훨씬 빈번하게 일어날 것으로 예상되므로   
rw_semaphore를 사용하여 동기화를 구현하였다.
  
이 락은 여러 읽기 작업을 병렬로 허용하면서도, 쓰기 시에는 배타적 접근을 보장한다.   
spinlock은 유저 공간 접근 중 sleep이 발생할 수 있어 사용할 수 없고,   
mutex는 읽기 또한 직렬화되므로 이 상황에서는 적합하지 않다.

다만 rw_semaphore를 사용하더라도 동일한 파일 디스크립터에 대한 읽기 작업은 병렬로 수행되지 않는다.   
이는 VFS 계층이 foo_read()를 호출하기 전에 이미 락을 걸어 접근을 직렬화하기 때문이다.   
이러한 락은 일반 파일에만 적용되며, foo 역시 일반 파일이므로 동일하게 적용된다.   
이 메커니즘은 I/O 수행과 f_pos 갱신을 원자적으로 보장하기 위한 것이다.

또한 이러한 락이 없더라도, I/O 중 f_pos가 다른 스레드에 의해 변경되는 일은 발생하지 않는다.    
그 이유는 VFS 계층이 f_pos를 직접 전달하지 않고,   
그 값을 지역 변수에 복사한 뒤 해당 지역 변수의 포인터를 넘겨주기 때문이다.

#### 참고 자료
- [DebugFS](https://docs.kernel.org/filesystems/debugfs.html)

## ex08
