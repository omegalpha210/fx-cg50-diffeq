[English](README.md) | 한국어

<p align="center">
  <img src="docs/images/diffeq-icon.png" width="184" height="128" alt="해 곡선과 기울기장을 그린 DIFFEQ 자체 아이콘">
</p>

# DIFFEQ for CASIO fx-CG50

계산기에서 미분방정식을 풀고, 그리고, 탐색하세요.
DIFFEQ는 **CASIO fx-CG50용 네이티브 애드인**입니다. 미분방정식의 수치해를
컬러 그래프·기울기장·TRACE·G-Solve·표로 살펴볼 수 있습니다.

**공개 베타 · v0.9.0-beta.3 · [MIT 라이선스](LICENSE)**

**[베타 다운로드](https://github.com/omegalpha210/fx-cg50-diffeq/releases/tag/v0.9.0-beta.3)**
· [전체 릴리스](https://github.com/omegalpha210/fx-cg50-diffeq/releases)
· [버그 신고](https://github.com/omegalpha210/fx-cg50-diffeq/issues/new/choose)

![y'=1-y^2의 두 해 곡선과 옅은 파란색 화살표 기울기장](docs/images/graph-slope-field.png)

*이 페이지의 화면은 실제 DIFFEQ 앱 렌더러를 호스트 테스트 환경에서 실행해 생성했습니다.
계산기 사진이나 CPU 에뮬레이터 캡처가 아닙니다. 프로젝트 소유자가 실제 fx-CG50에서
1차·2차의 핵심 작업 흐름을 시험했으며, 이번 베타의 최신 UI와 기기 동작은 추가 시험이 필요합니다.*

휴대폰에서는 이미지를 눌러 원래 크기로 자세히 볼 수 있습니다.

## 식 입력부터 그래프까지

**Equation → Initial Conditions → Solver Parameters → Graph**

| 1. 식 입력 | 2. 초기조건 지정 |
|---|---|
| ![일반 1차 식 편집기에 입력한 1-y^2](docs/images/equation-entry.png) | ![x0=0, y0={0,0.5} 초기조건](docs/images/initial-conditions.png) |
| 방정식 종류를 선택하고 우변을 입력합니다. | 1차에서는 y0 목록의 값마다 별도 해를 그립니다. |

| 3. 계산 설정 | 4. 그래프 탐색 |
|---|---|
| ![적분 구간 -3부터 3, h=0.1, Step=1, SF=12인 Parameters](docs/images/solver-parameters.png) | ![기울기장을 끈 상태의 magenta와 cyan 해 곡선](docs/images/graph-solution.png) |
| 적분 구간, h와 기울기장 밀도를 설정합니다. | GRAPH를 눌러 계산한 뒤 이동·확대·TRACE·표를 사용합니다. |

단계 이동은 **F6 NEXT**, 계산은 **F6 GRAPH**입니다. 일반 필드를 편집하지 않고
선택만 한 상태에서는 어느 행에서든 **EXE**가 같은 화면의 주 동작을 실행합니다.

## 그래프를 더 자세히 살펴보기

| TRACE | G-Solve |
|---|---|
| ![x와 y 좌표, NORMAL FAST FASTER 버튼이 있는 TRACE 커서](docs/images/graph-trace.png) | ![첫 번째 해의 y절편 x=0, y=0을 찾은 G-Solve](docs/images/graph-gsolve.png) |
| 해를 따라 이동하고 곡선을 전환합니다. | 근·극값·절편·두 곡선의 교점을 찾습니다. |

| 수치 표 | 기울기장 |
|---|---|
| ![0을 중심으로 x와 두 해를 표시하는 TOP BTM MID STAT 표](docs/images/table-view.png) | ![y'=1-y^2의 화살표 기울기장과 두 해](docs/images/graph-slope-field.png) |
| x 열을 유지하며 해를 넘겨 보고 STAT용 CSV로 내보냅니다. | Parameters에서 밀도, SET에서 스타일과 색을 바꿉니다. |

## 주요 기능

- **7가지 방정식 유형:** 변수분리형·선형·Bernoulli·일반 1차, 선형 2차, 일반 N차,
  연립 미분방정식. 차수와 시스템 크기는 **1~9**이며 N차→시스템 변환과 두 상태의 위상 궤적을 지원합니다.
- 초기조건에서 양방향으로 적분하는 **고전적 RK4**. 1차는 공통 x0에서 **최대 10개 y0**를
  입력할 수 있습니다. 고차·시스템 UI는 모든 상태 초기값을 갖춘 하나의 벡터를 입력합니다.
- **1차 기울기장:** SF 0~100, Segment/Arrow와 옅은 색 6종. 기본은 Arrow / Pale Blue입니다.
- **V-Window·이동·확대**, 해 색상 6종과 종속변수별 공통 ON/OFF.
- **TRACE:** NORMAL / FAST / FASTER 이동, 곡선 전환, x 지정, X/Y 화면 자동 추종과 설정된 적분 구간 양끝 이동.
- **G-Solve:** ROOT, MAX, MIN, Y-ICPT, ICPT, X-CAL, Y-CAL.
- **Table:** x 오름차순, TOP / BTM / MID, 고정 x 열과 해 열 가로 이동.
  **STAT 호환 CSV**로 최대 998개 데이터 행을 내보냅니다.
- 명시적 **SAVE / RCL**, 복구 가능한 저장 슬롯과 이전 세션 변환.

TRACE 속도 버튼은 **노랑 / Bright Green / Cyan** 배경과 검정 글씨이며 선택한 모드에
테두리가 생깁니다. NORMAL=1×Xdot, FAST=2×, FASTER=3×입니다. F5 LEFT/F6 RIGHT는 곡선과
속도를 유지하며 설정된 Solver Xrange 양끝으로 이동합니다. 끝점에 도달하는 것만으로 미리
적분하지 않고 실제 계산 범위 밖 이동 요청에서 확장합니다. X/Y 추적은 폭·높이와 solver 설정을 유지합니다.

Graph Settings의 Grid/Axis Label은 LEFT/RIGHT로 토글하고 F1/F2는 비어 있습니다.
1차 목록은 **191자** 입력 한도를 유지하며 개수·길이 오류를 구분합니다. Table은 x를 고정한 채
최대 10개 해 열을 탐색할 수 있습니다.

## 화면 속 예제 실행하기

**1st → Others**에서 일반 1차 식과 초기조건을 입력합니다.

```text
y' = 1-y^2
x0 = 0
y0 = {0,0.5}
h = 0.1
```

V-WIN은 Xmin `-3`, Xmax `3`, Xscale `1`, Ymin `-1.5`, Ymax `1.5`, Yscale `0.5`로
설정합니다. Xdot은 자동으로 갱신됩니다. Parameters의 Step `1`, SF `12`, Max steps
`20000`은 그대로 둡니다. 자동 적분 구간은 `-3`부터 `3`입니다.
두 해는 오른쪽에서 y=1, 왼쪽에서 y=-1에 가까워집니다.
**SF=0**으로 바꾸면 기울기장 없이 해만 표시한 화면을 재현할 수 있습니다.

`y`는 ALPHA+SUB, `{`와 `}`는 SHIFT+곱셈/나눗셈, 목록 구분자는 물리 comma 키입니다.
곱셈은 `2*y`처럼 명시하며 삼각함수는 항상 radian을 사용합니다.
더 자세한 설명은 [전체 사용 설명서](docs/USER_GUIDE.md)를 참고하세요.

## 계산기에 설치하기

1. [현재 베타 릴리스](https://github.com/omegalpha210/fx-cg50-diffeq/releases/tag/v0.9.0-beta.3)를 엽니다.
2. **DIFFEQ.g3a**를 받습니다. 다운로드 확인용 `SHA256SUMS.txt`도 제공됩니다.
3. fx-CG50을 USB로 연결하고 USB Flash 모드를 선택한 뒤 컴퓨터에서 계산기 드라이브를 엽니다.
4. `DIFFEQ.g3a`를 드라이브 **최상위**에 복사합니다. `@MainMem` 폴더 안에 넣지 않습니다.
5. 드라이브를 안전하게 꺼내고 USB 연결을 종료합니다.
6. 계산기 Main Menu에서 **DIFF EQ**를 실행합니다.

[CASIO 공식 애드인 설치 안내](https://edu.casio.com/content/dam/casio/global/edu-casio-com/download/files/fx-cg50-series/Inst_Users_Guide.pdf)에 따른 절차입니다.
계산기에는 `.g3a`만 있으면 됩니다. 업데이트 전에 기존 세션 파일을 백업하세요.
이번 버전은 v7 형식으로 저장하며 같은 기기의 v3/v4/v5/v6 파일을 읽습니다. 구버전 앱은
새 저장 파일을 거부할 수 있고, 일부 이전 설정은 변환됩니다. [업그레이드 안내](docs/release/RELEASE_NOTES.md)를 확인하세요.

## 핵심 조작

| 화면·상태 | 키 |
|---|---|
| Main | 숫자 1~4 또는 UP/DOWN+EXE로 유형 선택, F5 RCL, F6 SAVE |
| 일반 필드 선택 | UP/DOWN으로 선택, LEFT/RIGHT로 편집 시작, EXE로 NEXT/GRAPH/DONE/OPEN |
| 편집 중 | EXE는 확정 후 다음 필드 선택, 마지막 행은 머묾. EXIT는 확정 후 같은 행 선택 |
| Equation | EDIT에서 FUNC 사용, 필요한 모드에 VAR 표시. EXIT는 열린 token bar부터 닫음 |
| OUTPUT | LEFT/RIGHT ON/OFF, F3 COLOR, F4 INIT, F6 DONE. EXE는 출력 행 순서로 이동 |
| Parameters | F3 V-WIN, F4 OUTPUT, F5 SET, F6 GRAPH |
| Graph | 방향키 이동, F1 TRACE, F2 ZOOM, F3 V-WIN, F4 TABLE, F5 G-SLV, F6 magenta PREV |
| TRACE | LEFT/RIGHT 이동, UP/DOWN 곡선 전환, F1 x=, F2~F4 속도, F5/F6 설정 구간 양끝, EXIT 복귀 |
| Table | UP/DOWN 페이지, LEFT/RIGHT 열 이동, TOP/BTM/MID, F5 STAT |
| 세션 | SAVE는 명시적 저장, RCL은 마지막 계산 또는 저장 파일 복원, MENU는 계산기 OS 복귀 |

SF는 scalar 1차 네 모드의 Parameters에만 표시합니다. 고차/SYS는 N-th1·SYS1을 포함해
SF를 숨기며 INIT 후에도 그 값을 보존합니다. 1차 INIT는 SF=12로 복구합니다.
기울기장 스타일과 색은 Graph Settings에서 설정합니다.

## 소스에서 빌드하기

호환되는 fxSDK/gint가 이미 설치되어 있고 PATH에 설정된 환경에서 실행합니다.

```sh
fxsdk build-cg -j8
python3 tools/verify_g3a.py dist/DIFFEQ.g3a
./tools/test.sh
```

C11/CMake를 사용하며 gint ≥2.11이 필요합니다. 개발·릴리스 빌드는 fxSDK/gint 2.11.0,
SH GCC 14.1.0, binutils 2.42, fxlibc 1.5.1과 고정한 OpenLibm SH port에서 검증했습니다.
이 목록은 시험한 버전이며 모두 영구적인 최소 요구사항이라는 뜻은 아닙니다.
호스트 스크립트는 Clang·CMake·Python 3를 사용하며 UBSan이 기본으로 활성화됩니다.

기존 SDK 사용법, 릴리스 검사와 이미지 재현은 [DEVELOPMENT.md](DEVELOPMENT.md)에 있습니다.
릴리스 바이너리는 공개 태그의 소스로 빌드합니다. 빌드 시각이 파일에 들어가므로 나중에
같은 소스를 다시 빌드한 파일의 해시는 달라질 수 있습니다.

## 수치 계산과 호환성

**대상 기기는 CASIO fx-CG50입니다.** 다른 기종과 에뮬레이터는 검증하지 않았습니다.

`h`는 RK4 적분 간격입니다. 작게 하면 정확도가 좋아질 수 있지만 계산량이 늘어나므로
여러 h에서 결과를 비교하세요. 사전 계산량 검사와 Max steps가 과도한 계산을 제한합니다.
고정 간격 RK4는 적응형·강성 전용 해법이 아니며, 급격한 변화나 특이점에서 수치 한계에 도달할 수 있습니다.

NaN/Inf와 절댓값 `1e100` 초과 영역은 그리지 않습니다. 계산된 유효 구간은 유지하며,
실패한 지점 너머로 해를 임의 연결하지 않습니다. G-Solve는 표본 사이의 특징을 놓칠 수 있습니다.
TRACE는 제한된 표본과 보간을 사용하고 Table은 값을 다시 계산할 수 있습니다.
CSV는 STAT에서 직접 가져와야 하며 OS List를 자동으로 쓰지 않습니다.
[수치 안전성](docs/SOLVER_SAFETY_AUDIT.md)과 [릴리스 검증](docs/ACCEPTANCE.md)을 참고하세요.

**HARDWARE TEST REQUIRED:** 최신 LCD 배치·색상, 키 반복·blink 타이밍,
MENU/Fugue 복귀, 실제 SAVE/RCL·STAT은 계산기에서 재시험해야 합니다.
[하드웨어 체크리스트](docs/HARDWARE_RETEST.md)는 이를 호스트 PASS와 구분합니다.

## 기여와 버그 신고

계산기·OS·베타 버전, 식과 초기값, solver/window 설정, 누른 키 순서와 기대·실제 결과를
함께 알려주세요. DIFFEQ 사진·영상도 도움이 됩니다. 첨부 전 개인정보를 제거해 주세요.
[이슈 등록](https://github.com/omegalpha210/fx-cg50-diffeq/issues/new/choose) 또는
[기여 안내](CONTRIBUTING.md)를 이용할 수 있습니다.

## 라이선스와 감사

프로젝트 자체 코드·문서·원본 아이콘은 [MIT 라이선스](LICENSE)입니다.
의존성은 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)에 기록된 각자의 조건을 유지합니다.
fxSDK/gint, fxlibc, OpenLibm, GNU toolchain 기여자들에게 감사드립니다.
호스트 font/key 자료는 고지된 gint revision에서 가져왔습니다.
[이미지 출처와 재현 방법](docs/images/README.md)도 공개합니다.

Algebra FX 2.0의 DIFF EQ 앱에서 영감을 받았습니다. 참조 매뉴얼과 매뉴얼 화면은
배포하지 않습니다. CASIO와 제휴하거나 CASIO의 승인을 받은 프로젝트가 아니며,
CASIO와 제품명 상표는 각 권리자에게 있습니다.
