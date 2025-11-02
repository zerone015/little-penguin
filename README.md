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
커널 모듈은 커널 실행 중에 동적으로 적재될 수 있는 드라이버이다.   
핫플러그 장치를 지원하려면 이러한 동적 적재 방식이 필수적이며,   
내장 드라이버 방식만 존재한다면 장치를 연결할 때마다 커널을 재빌드해야 한다.   

모듈은 insmod / rmmod 명령을 통해 즉시 로드 및 언로드할 수 있으며,   
이로 인해 필요한 기능만 동적으로 추가할 수 있어 커널 이미지를 가볍게 유지할 수 있다.   

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
- linux kernel source tree  
 
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
