[English](README.md) | 한국어

<p align="center">
  <img src="docs/images/diffeq-icon.png" width="184" height="128" alt="해 곡선과 기울기장을 그린 DIFFEQ 자체 아이콘">
</p>

# DIFFEQ for CASIO fx-CG50

계산기에서 미분방정식을 풀고, 그리고, 탐색하세요.
DIFFEQ는 **CASIO fx-CG50용 네이티브 애드인**입니다. 미분방정식의 수치해를
컬러 그래프·기울기장·TRACE·G-Solve·표로 살펴보고, 2변수 시스템의 위상을 분석할 수 있습니다.

**공개 베타 · v0.12.0-beta.7 · [MIT 라이선스](LICENSE)**

**[베타 다운로드](https://github.com/omegalpha210/fx-cg50-diffeq/releases/tag/v0.12.0-beta.7)**
· [전체 릴리스](https://github.com/omegalpha210/fx-cg50-diffeq/releases)
· [버그 신고](https://github.com/omegalpha210/fx-cg50-diffeq/issues/new/choose)

![y'=1-y^2의 두 해 곡선과 옅은 파란색 화살표 기울기장](docs/images/graph-slope-field.png)

*이 페이지의 화면은 실제 DIFFEQ 앱 렌더러를 호스트 테스트 환경에서 실행해 생성했습니다.
계산기 사진이나 CPU 에뮬레이터 캡처가 아닙니다. 프로젝트 소유자가 실제 fx-CG50에서
1차·2차의 핵심 작업 흐름을 시험했으며, 이번 베타의 최신 UI와 기기 동작은 추가 시험이 필요합니다.*

휴대폰에서는 이미지를 눌러 원래 크기로 자세히 볼 수 있습니다.

## 방정식 종류 선택

| Main: 2열 × 3행 | 1차 유형: 2열 × 2행 |
|---|---|
| ![1st를 선택한 DIFF EQ 타일 메뉴와 낮은 RECALL·SAVE 행](docs/ui-review/tiles-main-first.png) | ![Separable을 선택한 네 개의 1차 유형 타일](docs/ui-review/tiles-subtype-first.png) |

방향키로 행·열을 따라 선택하고 숫자 단축키 또는 **EXE/F6 OPEN**으로 엽니다.
Main의 위 네 타일과 subtype 타일은 같은 184×58 크기이며 RECALL/SAVE는 낮은 텍스트 행입니다.
8개 그래프 도상은 작은 const 좌표로 직접 그린 원본입니다. [아이콘·재현 방법](assets/menu/README.md)

## 식 입력부터 그래프까지

**Equation → Initial Conditions → Solver Parameters → Graph**

| 1. 식 입력 | 2. 초기조건 지정 |
|---|---|
| ![일반 1차 식 편집기에 입력한 1-y^2](docs/images/equation-entry.png) | ![x0=0, y0={0,0.5} 초기조건](docs/images/initial-conditions.png) |
| 방정식 종류를 선택하고 우변을 입력합니다. | 1차에서는 y0 목록의 값마다 별도 해를 그립니다. |

| 3. 계산 설정 | 4. 그래프 탐색 |
|---|---|
| ![RK45, h0=0.1, RelTol=1e-6, AbsTol=1e-9인 Parameters](docs/images/solver-parameters.png) | ![기울기장을 끈 상태의 magenta와 cyan 해 곡선](docs/images/graph-solution.png) |
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

## 2변수 시스템의 Phase Portrait

| 2변수 시스템 입력 | 위상 궤적 탐색 |
|---|---|
| ![조화진동자 y1'=y2, y2'=-y1을 입력한 시스템 편집기](docs/images/phase-system-input.png) | ![정규화한 방향 화살표와 조화진동자의 위상 궤적](docs/images/phase-field.png) |
| SYS → 2를 선택합니다. 화면의 초기값은 (y1,y2)=(1,0)입니다. | 계산 후 F4 VIEW → F2 PHASE로 이동합니다. |

![수치 nullcline, 원점 부근 평형점과 Center / Neutral candidate 선형화 분류](docs/images/phase-equilibrium.png)

**F5 ANLYS**에서 **F1 FIELD**, **F2 NULL**, **F3 EQPT**, **F4 INFO**를 사용합니다.
벡터장·nullcline을 켜거나 끄고, 평형점을 찾은 뒤 수치 Jacobian과 고윳값을 확인합니다.
LEFT/RIGHT는 찾은 평형점을 순회하고 EXIT는 분석 메뉴에서 나갑니다.
N1(빨강)은 `f1=0`, N2(파랑)는 `f2=0`입니다. **F4 VIEW → F1 TIME**으로 시간 그래프에
돌아가며, VIEW 안의 **F3 TABLE**로 표를 엽니다. Time과 Phase의 V-Window는 별도로 유지됩니다.

평형점 검색은 자율 시스템에서만 가능합니다. 비자율 시스템도 위상 궤적을 표시하며,
벡터장·nullcline은 화면에 안내한 기준 **x=x0**에서 평가합니다. 안정성 분류는 **국소 선형화**의
결과입니다. Center / Neutral candidate는 비선형·전역 안정성을 확정하지 않습니다.
검색에서 점을 놓칠 수 있고, 불확실하거나 미분을 구할 수 없는 경우 Inconclusive/Unavailable로 표시합니다.

두 그래프는 **최대 258점의 궤적 캐시**를 공유합니다. 호환되는 캐시 투영과 PHASE 이동·확대는
재적분을 피하지만 TIME AUTO 범위 변경은 solver를 다시 실행할 수 있습니다.
촘촘한 특징은 보관된 표본 해상도를 넘어설 수 있습니다. Phase TRACE는
이 캐시를 사용하고 계산된 시간 구간 안에서 이동합니다. [수치 방법과 한도](docs/PHASE_NUMERICS.md)를 참고하세요.
**HARDWARE TEST REQUIRED:** 새 RK45 계산/취소 및 stack high-water, 새 Phase 화면과 조작은 호스트에서 시험했으며 실제 기기 검증이 필요합니다.

## Solver Methods

| 방식 | step 제어 | 사용 목적 |
|---|---|---|
| **Classical RK4** (기본값) | 고정 h, Step으로 출력 간격 조절 | 기존 결과 유지, h를 바꿔 비교 |
| **Dormand–Prince RK45** | embedded local error에 따른 adaptive h | non-stiff 해의 변화에 맞춰 간격 자동 조절 |

Parameters 세 번째 **Method** 행에서 LEFT/RIGHT로 전환합니다. 계산은 GRAPH에서 시작합니다.
RK45는 **h0 / RelTol / AbsTol / Max steps**를 표시하며 기본값은 .1 / 1e-6 / 1e-9 / 20000입니다.
Max steps는 거절을 포함한 시도 횟수입니다. Step은 숨기고 SF는 scalar 1차에서만 표시합니다.
두 방식이 h 값을 공유하며 tolerance와 숨겨진 Step을 보존합니다. INIT는 Method를 유지하고
해당 방식의 설정을 초기화합니다. v3~v8 저장은 RK4, v9는 저장된 Method로 읽으며 모든 구버전의 Event는 OFF입니다.

모든 방정식 모드·Graph·TRACE·Table·G-Solve·Phase에서 RK45를 사용할 수 있습니다.
Table/G-Solve는 요청 x에 직접 도착하도록 적분합니다. TIME/Phase TRACE 이동은
기존 캐시의 화면용 선형 보간이며 점 사이에서 tolerance 정확도를 보장하지 않습니다.
출력 격자는 TIME Xdot을 기준으로 내부 adaptive step과 별도로 정합니다.
**RK45는 explicit adaptive 방식이며 stiff ODE 전용 solver가 아닙니다.**
강성이나 엄격한 tolerance에서는 Work limit/Step underflow에 도달할 수 있습니다.
[계수·안전장치·벤치마크·메모리](docs/RK45_NUMERICS.md)를 참고하세요.

## Event Detection / Solver Diagnostics

Parameters **F2 ADV → F1 EVENT**에서 하나의 `E(x,state)=0` 조건을 설정합니다.
**ANY / RISING / FALLING**은 backward에서도 x 증가 기준이며 **MARK / STOP**을 선택합니다.
MARK는 계속 적분하며 최대 **32개** marker를 표시하고 그 이후 hit도 계속 셉니다.
STOP은 IC·방향마다 refined root에서 종료합니다. Table/TRACE는 **END: Event**를 표시하고
G-Solve도 유효한 해의 범위 안에서 동작합니다.

**ADV → F2 INFO**는 RK4/RK45의 상태, refinement를 포함한 실제 수치 작업량, step 크기와
Event 합계를 읽기 전용으로 표시합니다. UP/DOWN으로 읽고 EXIT로 복귀하며 계산·파일 쓰기는 없습니다.
Parameters **F1 INIT**는 Method·Event·V-Window를 유지하며 solver 설정을 초기화합니다.

| Event Settings | Solver Diagnostics (RK45) |
|---|---|
| ![Event ON, y-10, RISING, STOP 설정](docs/images/event-settings.png) | ![RK45 허용오차, 수락·거절·시도 횟수와 실제 RHS 호출 수](docs/images/solver-diagnostics.png) |

기본 Event는 OFF입니다. accepted step 하나 안의 여러 crossing은 놓칠 수 있으며 root 정확도는
수치해 오차의 영향을 받습니다. [알고리즘·벤치마크·한계](docs/EVENTS.md) · [사용 설명서](docs/USER_GUIDE.md)

## 주요 기능

- **7가지 방정식 유형:** 변수분리형·선형·Bernoulli·일반 1차, 선형 2차, 일반 N차,
  연립 미분방정식. 차수와 시스템 크기는 **1~9**이며 N차→시스템 변환과 두 상태의 위상 궤적을 지원합니다.
- 초기조건에서 양방향으로 적분하는 **고전적 RK4 또는 적응형 Dormand–Prince RK45**. 1차는 공통 x0에서 **최대 10개 y0**를
  입력할 수 있습니다. 고차·시스템 UI는 모든 상태 초기값을 갖춘 하나의 벡터를 입력합니다.
- **1차 기울기장:** SF 0~50, Segment/Arrow와 옅은 색 6종. 기본은 Arrow / Pale Blue입니다.
- **2D SYS Phase:** 정규화 벡터장, 수치 nullcline, 최대 16개 평형점 후보,
  Jacobian·고윳값과 국소 선형 안정성 분류를 제공합니다.
- **V-Window·이동·확대**, 해 색상 6종과 종속변수별 공통 ON/OFF.
- **TRACE:** 가로 화면 고정·Y만 자동 추종, NORMAL / FAST / FASTER, 곡선 전환, 진입 커서 INIT와 유효 경계 내 끝점 이동.
- **G-Solve:** ROOT, MAX, MIN, Y-ICPT, ICPT, X-CAL, Y-CAL.
- **Table:** x 오름차순, TOP / BTM / MID, 고정 x 열과 해 열 가로 이동.
  **STAT 호환 CSV**로 최대 998개 데이터 행을 내보냅니다.
- 명시적 **SAVE / RCL**, 복구 가능한 저장 슬롯과 이전 세션 변환.

TRACE는 **진입 시 가로 화면 범위 ∩ 선택 해의 연결된 유효 수치 구간** 안에서 이동합니다.
TRACE 조작으로 X 화면을 옮기거나 계산 구간을 확장하지 않습니다. 유효한 점의 Y만 추종하며
Y span·양축 scale·Xdot·solver 설정을 유지합니다.
F1 **INIT**(노랑/검정)는 진입 커서·곡선을 복구하고 속도는 유지합니다.
F2 **NORMAL**(주황/검정), F3 **FAST**(Bright Green/검정), F4 **FASTER**(Cyan/검정)는
진입 Xdot의 1×/2×/3× 간격입니다. 반복 주기와 선택 테두리는 유지합니다.
F5 LEFT/F6 RIGHT는 설정한 적분 끝점을 향하되 현재 화면에서 도달 가능한 유효 경계에 멈춥니다.
invalid gap이나 Event STOP 너머로 연결하지 않습니다.

PHASE는 가로 상태 범위(SYS2의 y1)를 고정하고 y2만 추종합니다. integration x 순서로
궤적을 따라가다가 화면 밖 보관 표본 전에 멈추며 가짜 경계점으로 투영하지 않습니다.
아주 좁은 가로 범위에 보관 표본이 없으면 TRACE를 사용할 수 없습니다.
**EXIT 이후 일반 Graph·ZOOM·G-Solve 메뉴의 pan 및 기존 안전한 확장은 유지합니다.**

모든 numerical/domain 경고는 plot 좌측 최상단의 같은 위치에 빨간 normal 글자와
글자 크기에 맞춘 작은 불투명 흰 배경으로 표시합니다. 유효 구간 TRACE/G-Solve는 계속 사용하며
정상 Event STOP은 중립색입니다. [경고·TRACE 실제 렌더러 화면](docs/ui-review/tiles-overview.png)

Graph Settings는 **F1 INIT**로 Grid·Axis Label·기울기장 스타일/색을 초기화합니다.
Style은 LEFT/RIGHT로 바꾸고 F1 INIT는 Style을 포함한 모든 행에서 작동합니다. F2는 비어 있습니다.
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

Parameters **F3 V-WIN**에서 Xmin `-3`, Xmax `3`, Xscale `1`, Ymin `-1.5`, Ymax `1.5`, Yscale `0.5`로
설정합니다. Xdot은 자동으로 갱신됩니다. 기존 RK4 그래프 예제는 Method RK4, Step `1`, SF `12`, Max steps
`20000`은 그대로 둡니다. 자동 적분 구간은 `-3`부터 `3`입니다.
두 해는 오른쪽에서 y=1, 왼쪽에서 y=-1에 가까워집니다.
**SF=0**으로 바꾸면 기울기장 없이 해만 표시한 화면을 재현할 수 있습니다.

`y`는 ALPHA+SUB, `{`와 `}`는 SHIFT+곱셈/나눗셈, 목록 구분자는 물리 comma 키입니다.
곱셈은 `2*y`처럼 명시하며 삼각함수는 항상 radian을 사용합니다.
더 자세한 설명은 [전체 사용 설명서](docs/USER_GUIDE.md)를 참고하세요.

## 계산기에 설치하기

1. [현재 베타 릴리스](https://github.com/omegalpha210/fx-cg50-diffeq/releases/tag/v0.12.0-beta.7)를 엽니다.
2. **DIFFEQ.g3a**를 받습니다. 다운로드 확인용 `SHA256SUMS.txt`도 제공됩니다.
3. fx-CG50을 USB로 연결하고 USB Flash 모드를 선택한 뒤 컴퓨터에서 계산기 드라이브를 엽니다.
4. `DIFFEQ.g3a`를 드라이브 **최상위**에 복사합니다. `@MainMem` 폴더 안에 넣지 않습니다.
5. 드라이브를 안전하게 꺼내고 USB 연결을 종료합니다.
6. 계산기 Main Menu에서 **DIFF EQ**를 실행합니다.

[CASIO 공식 애드인 설치 안내](https://edu.casio.com/content/dam/casio/global/edu-casio-com/download/files/fx-cg50-series/Inst_Users_Guide.pdf)에 따른 절차입니다.
계산기에는 `.g3a`만 있으면 됩니다. 업데이트 전에 기존 세션 파일을 백업하세요.
이번 버전은 v10 형식으로 저장하며 같은 기기의 v3~v9 파일을 읽습니다. 구버전 앱은
새 저장 파일을 거부할 수 있고, 일부 이전 설정은 변환됩니다. [업그레이드 안내](docs/release/RELEASE_NOTES.md)를 확인하세요.

## 핵심 조작

| 화면·상태 | 키 |
|---|---|
| Main | 숫자 1~6: 1st/2nd/N-th/SYS/RCL/SAVE; 방향키로 행·열 순환 선택, EXE 또는 F6 OPEN 진입; F1~F5 비움 |
| 일반 필드 선택 | UP/DOWN 순환 선택, LEFT/RIGHT로 편집 시작, EXE로 NEXT/GRAPH/DONE/OPEN |
| 편집 중 | EXE는 확정 후 다음 필드 선택, 마지막 행은 머묾. EXIT는 확정 후 같은 행 선택 |
| Equation | F1 INIT, EDIT에서 F2 FUNC/F3 VAR(지원 모드만). EXIT는 열린 token bar부터 닫음 |
| OUTPUT | LEFT/RIGHT는 visibility 행의 ON/OFF (IC 색상 행에서는 무동작), F1 INIT, F3 COLOR, F6 DONE. EXE는 출력 행 순서로 이동 |
| Parameters | F1 INIT; F2 ADV → EVENT/INFO; Method: LEFT/RIGHT로 RK4/RK45 전환; F3 V-WIN, F4 OUTPUT, F5 SET, F6 GRAPH |
| Graph (2D SYS 제외) | 방향키 이동, F1 TRACE, F2 ZOOM, F3 V-WIN, F4 TABLE, F5 G-SLV, F6 노랑/검정 INIT; EXIT로 Parameters 복귀 |
| 2D SYS Graph | F4 VIEW → F1 TIME / F2 PHASE / F3 TABLE, Phase에서는 F5 ANLYS; F6 INIT는 선택 view 유지 |
| Phase 분석 | F1 FIELD, F2 NULL, F3 EQPT, F4 INFO, LEFT/RIGHT 평형점 순회, EXIT 복귀 |
| TIME TRACE | LEFT/RIGHT 이동, UP/DOWN 곡선 전환, F1 진입 커서 INIT, F2~F4 속도, F5/F6 화면 내 유효 경계로 제한한 끝점, EXIT 복귀 |
| Phase TRACE | 보관한 궤적의 x·y1·y2 표시, 가로 상태 범위 고정·Y만 추종, 시간 구간 확장 없음 |
| Table | UP/DOWN 페이지, LEFT/RIGHT 열 이동, TOP/BTM/MID, F5 STAT |
| 세션 | SAVE 확인: F5 NO/EXIT 취소, F6 YES/EXE 한 번 저장; RCL은 마지막 계산 또는 파일 복원; MENU는 OS 복귀 |

SF는 scalar 1차 네 모드의 Parameters에만 표시합니다. 고차/SYS는 N-th1·SYS1을 포함해
SF를 숨기며 INIT 후에도 그 값을 보존합니다. 1차 INIT는 SF=12로 복구합니다.
기울기장 스타일과 색은 Graph Settings에서 설정합니다.

입력 화면은 **Equation 1/3 → IC 2/3 → Parameters 3/3**을 표시합니다.
IC/Parameters에서 EXIT로 이전 단계로 돌아가며, V-WIN은 Parameters와 Graph에서만 엽니다.
INIT는 노란 배경/검정 글씨로 해당 화면의 설정만 초기화합니다. Parameters는 Method·Event를
유지하고, V-WIN은 창의 기하 설정, Graph Settings는 Grid/Label/기울기장 스타일·색,
Output은 종속변수 ON/OFF·색을 초기화합니다. ADV는 검정 배경/흰 글씨이며 계산 없이 유틸리티를 엽니다.
SELECT 목록만 끝에서 순환하고 편집 커서·Graph/TRACE/Phase·Table 이동은 기존 동작을 유지합니다.
Main은 옅은 색 타일·청록/파랑 선택 테두리와 빨간 MENU 안내를 표시합니다. 일반 EXE OPEN/NEXT/GRAPH
안내는 숨기고 EDIT·palette·BOX·곡선 선택에 필요한 EXE는 normal-weight 파랑으로 한 번씩 그립니다.
AUTO/MAN·RK45 h0는 유지합니다. **TIME/PHASE**는 SYS2 VIEW가 있을 때만, **EVT**는 Event
활성 시 ODE 유형과 관계없이 표시합니다.

**Graph F6 INIT**는 **V-WIN F1 INIT와 같은 factory 창 초기화**를 사용하고 TIME/PHASE 선택과 manual solver
설정을 유지합니다. 호환 캐시는 재사용하며 캐시/Event 기록의 범위가 부족하면 기존 안전한 redraw를
사용합니다. **ZOOM F4 ORIG**는 factory 창입니다. **F2 ZOOM → F5 BOX**는 중앙에서 시작하며,
방향키 4px 이동 → EXE Point1 고정 → Point2 이동 → EXE로 가로·세로 최소 6px 영역을 확정합니다.
Pale stipple 아래 곡선이 보이고 어느 단계의 EXIT도 창을 바꾸지 않습니다. 두 view의 창은 독립입니다.

TRACE/G-Solve/BOX는 9px 검정 cross와 흰 중심을 공유하며 기존 2px 곡선 blink는 유지합니다.
G-Solve 선택은 좌측 최상단에 **UP/DOWN: SELECT GRAPH, EXE: SELECT**를 표시하며 EXE만
normal 파랑입니다. 활성 안내가 경고를 잠시 대체하고 종료 시 복구합니다.
**선택·결과에서 EXIT 한 번 → G-Solve submenu, 다음 새 EXIT → Graph**입니다.
HOLD는 계층을 건너뛰지 않고 scratch 계산 취소는 기존 그래프·trajectory 진단을 보존합니다.
결과 패널은 좌측 하단에 고정합니다. marker가 가려지면 X·scale·Y span·수치 결과를 유지하며
Y만 최소한 평행 이동합니다. 이미 보이는 점은 창을 움직이지 않고 결과 순회는 재계산하지 않습니다.

오래 걸리는 Table/Drawing 준비에는 파란 **Preparing Table...** 또는 **Drawing...**
header, `/ - \ |` spinner, **EXIT cancels** 행과 흰 본문을 사용하며 F-key strip을 숨깁니다.
약 156 ms 지연·최대 8 Hz로 표시하고 첫 canvas 이후에는 header만 갱신합니다.
빠른 작업·같은 Table 페이지에는 불필요한 화면을 띄우지 않습니다. EXIT는 임시 작업을
버리고 안정된 Graph로 돌아가며 최초 Drawing 취소는 Parameters로 돌아갑니다.
TRACE/G-Solve는 기존 하단 **CALCULATING...** 표시를 유지합니다.
취소한 Drawing은 Last calculation을 덮어쓰지 않습니다.

**복수 IC의 개별 색상:** Output의 IC1 y~IC10 y에서 F3 COLOR로 해당 곡선만 바꿉니다.
별도 **y (all ICs)** 행은 공통 ON/OFF이며 IC 행은 색상만 편집합니다.
INIT는 기존 palette로 복원하고 SAVE/RCL 및 IC 수 감소·재확장에서도 slot별 색을 보존합니다.
단일 IC는 기존 y 행을 사용합니다. SF는 기본 12, 범위 0~50이며 초과 입력을 거부하고
과거 저장값은 최대 50으로 정규화합니다. 저장 format은 바뀌지 않습니다.

유효한 Event STOP 끝점의 G-Solve 결과 누락과 RK45 Table의 dx 안내도 수정했습니다.
모든 방정식 mode·1~9차/변수·두 solver·1/2/5/10 IC와 소비자·상태·저장을 감사했습니다.
[전체 감사·검토 보류 사항](docs/FULL_AUDIT.md) ·
[실제 renderer 16개 화면](docs/ui-review/audit-overview.png).
Equation/IC F1 INIT는 해당 입력만 복구합니다. 미완성 draft는 NEXT에서 전체 검증하고 첫 오류를
선택합니다. IC numeric 값은 전체 성공 후에만 반영하며 미완성 IC draft는 runtime-only입니다.
빨간 domain/numerical END는 비치명 상태로 유지하고 유효 구간 TRACE/G-Solve를 계속 사용할 수 있습니다.
Output은 OFF에서도 선택 색의 선을 표시합니다.
[현재 전수 감사와 제약](docs/FULL_AUDIT.md), [UI 규칙](docs/UI_CONVENTIONS.md),
[BOX·변경 화면 모음](docs/ui-review/interaction-overview.png).

[Main·Subtype 6개 상태의 native/3× 캡처와 현재 TRACE·경고 화면](docs/ui-review/tiles-overview.png)

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
RK45는 추정 local error 제어를 추가하지만 두 방식 모두 전역 정확도나 모든 특이점 검출을 보장하지 않습니다.
수치적 pole 위치가 실제 위치와 조금 다를 수 있으며 실패한 prefix 너머에서 새 해를 시작하지 않습니다.

NaN/Inf와 절댓값 `1e100` 초과 영역은 그리지 않습니다. 계산된 유효 구간은 유지하며,
실패한 지점 너머로 해를 임의 연결하지 않습니다. G-Solve는 표본 사이의 특징을 놓칠 수 있습니다.
TRACE는 제한된 표본과 보간을 사용하고 Table은 값을 다시 계산할 수 있습니다.
CSV는 STAT에서 직접 가져와야 하며 OS List를 자동으로 쓰지 않습니다.
[수치 안전성](docs/SOLVER_SAFETY_AUDIT.md)과 [릴리스 검증](docs/ACCEPTANCE.md)을 참고하세요.

**HARDWARE TEST REQUIRED:** 새 RK45 계산/취소 및 stack high-water, Phase 표시·분석, 최신 LCD 배치·색상, 키 반복·blink 타이밍,
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
