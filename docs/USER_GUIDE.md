# DIFF EQ 사용 설명서 — v0.12.0-beta.8

기본 RK4와 TRACE·G-Solve·10개 초기값·SYS 2D Phase 기능을 보존하면서
Main/subtype native 타일, 고정 X/Y-only TRACE 및 공통 top-left 경고를 제공합니다. 기존 INIT/BOX/draft와 3단계 workflow는 유지합니다.
실제 fx-CG50의 새 경로 검증은 **HARDWARE TEST REQUIRED**입니다.

## 화면별 조작

| 화면 | F1 | F2 | F3 | F4 | F5 | F6 |
|---|---|---|---|---|---|---|
| 메인 | — | — | — | — | — | OPEN |
| Equation 1/3 | INIT | FUNC (EDIT만) | VAR (지원 모드 EDIT만) | — | — | NEXT |
| Initial Conditions 2/3 | INIT | — | — | — | — | NEXT |
| Solver Parameters 3/3 | INIT | ADV | V-WIN | OUTPUT | SET | GRAPH (빨강) |
| ADV submenu | EVENT | INFO | — | — | — | — |
| SAVE confirmation | — | — | — | — | NO | YES |
| V-WIN SELECT | INIT | — | — | — | — | DONE |
| Event (EDIT) | VAR (필요 시) | FUNC | — | — | — | DONE |
| Solver Info | — | — | — | — | — | — |
| Output 종속변수 | INIT | — | COLOR | — | — | DONE |
| Output 여러 IC 공통 ON/OFF | INIT | — | — | — | — | DONE |
| Output IC별 색 | INIT | — | COLOR | — | — | DONE |
| Graph Settings: Grid/Label | INIT | — | — | — | — | DONE |
| Graph Settings: Style | INIT | — | — | — | — | DONE |
| Graph Settings: Color | INIT | — | COLOR | — | — | DONE |
| FUNC 1쪽 | ABS | SINH | COSH | TANH | ASINH | 다음 |
| FUNC 2쪽 | ACOSH | ATANH | — | — | — | 이전 |
| 그래프 기본 화면 | TRACE | ZOOM | V-WIN | TABLE | G-SLV | INIT (노랑/검정) |
| SYS 2D TIME | TRACE | ZOOM | V-WIN | VIEW | G-SLV | INIT |
| SYS 2D PHASE | TRACE | ZOOM | V-WIN | VIEW | ANLYS | INIT |
| VIEW | TIME | PHASE | TABLE | — | — | — |
| Phase ANLYS | FIELD | NULL | EQPT | INFO | — | — |
| ZOOM 하위 메뉴 | IN | OUT | AUTO | ORIG | BOX | — |
| G-Solve 1쪽 | ROOT | MAX | MIN | Y-ICPT | ICPT | 다음 |
| G-Solve 2쪽 | Y-CAL | X-CAL | — | — | — | 이전 |
| Trace | INIT | NORMAL (주황/검정) | FAST | FASTER | LEFT | RIGHT |
| X-CAL / Y-CAL 숫자 입력 | — | — | — | — | — | — |
| Table | TOP | BTM | MID | — | STAT | GRAPH |

**[USER REQUESTED ADAPTATION]** 모든 모드는 Equation → F6 NEXT → IC → F6 NEXT →
Parameters → F6 GRAPH 순서입니다. 일반식은 Equation 상단, 실제 입력 행은 그 아래에 표시됩니다.
선택 상태에서 UP/DOWN은 이전/다음 입력 행을 선택하며 처음 UP은 마지막, 마지막 DOWN은 처음으로 순환합니다.
RK4/RK45와 SF 등 실제 보이는 행만 대상으로 합니다. EDIT 커서, Graph/Phase/ZOOM pan, TRACE,
G-Solve, Table 데이터/열, 2D palette 및 INFO scroll의 기존 이동 방식은 바꾸지 않습니다.
**편집 중이 아니면 EXE는 선택 행의 위치와 관계없이 F6 NEXT/GRAPH/DONE/OPEN을 실행합니다.**
따라서 기본 입력을 사용하면 Equation에서 EXE 세 번으로 Graph에 도달합니다.
LEFT는 편집 시작·커서 맨 앞, RIGHT는 편집 시작·커서 맨 뒤입니다. 직접 입력은 기존 값을 대체합니다.
편집 중 **EXE는 draft 확정 후 다음 행을 선택**하며, 다음 행의 편집까지 열지 않습니다.
마지막 행 편집에서는 첫 EXE가 확정 후 같은 행을 선택하고, 두 번째 EXE가 F6 동작을 실행합니다.
편집 중 **EXIT는 draft 확정 후 같은 행 선택**으로 돌아갑니다. 부모 화면에는 EXIT를 한 번 더 누릅니다.
Equation/IC는 미완성 draft를 허용하고 NEXT에서 검증합니다. 다른 숫자/설정 화면은 기존 확정 시 검증을 유지합니다. X/Y-CAL 숫자 prompt만 EXE 확정 / EXIT 즉시 취소를 따릅니다.

Equation·IC·Parameters·V-Window·차수/변수 개수의 일반 입력을 편집할 때 하단 한 줄은
`EXE: commit / next   EXIT: commit`입니다. 커서가 blink로 숨겨져도 EDIT 상태이면 유지합니다.
편집을 확정하면 해당 화면의 안내로 돌아갑니다. SELECT에는 EDIT 안내나 일반적인
`UP/DOWN: select`를 표시하지 않습니다. Table의 페이지 이동, TRACE의 곡선 전환,
G-Solve의 곡선 선택 등 특수 방향키 동작은 그대로입니다. 팝업·하위 메뉴의 전용 안내가
우선하고, 일반 폼에서는 EDIT 안내 → 화면별 필요한 안내 → 빈 줄 순서입니다.
Graph Settings의 ON/OFF·Style·Color 선택은 문자 입력 EDIT가 아니므로 전용 안내를 씁니다.
TRACE F1은 cursor INIT입니다. 숫자 x 조회는 G-Solve Y-CAL을 사용합니다.

Equation/IC NEXT는 모든 required field를 검증하고 첫 오류 필드에 머뭅니다.
Equation/IC F1은 INIT이며 Equation F3 VAR는 지원 모드 EDIT에서만 표시합니다. F3 V-WIN은 Parameters와 Graph에만 있습니다.
EXIT로 Parameters → IC → Equation으로 돌아갑니다. 각 title 오른쪽에 3/3 → 2/3 → 1/3을 표시합니다.
ADV·FUNC/VAR 등의 submenu와 Graph/보조 화면에는 progress를 표시하지 않습니다. OUTPUT과 SET은 Parameters에 있습니다.
보조 화면 EXIT는 진입 단계·선택 행으로 복귀합니다. SET은 중간 메뉴 없이 Graph settings를 바로 엽니다. Private Constants는 제거되었습니다. On/Off는 LEFT/RIGHT로 전환하고 EXE로 DONE을 실행합니다.
PREV는 magenta, NEXT는 cyan, V-WIN은 orange, SET은 bright green입니다.
실제 계산을 실행하는 F6 GRAPH는 red입니다. 모든 INIT는 Yellow/Black, ADV는 Black/White입니다.
단순 OPEN/NEXT/GRAPH와 같은 EXE 안내는 표시하지 않습니다. EDIT commit, palette SELECT, G-Solve 곡선 SELECT에 남긴 EXE는 한 번만 그리는 normal-weight Blue이고, 나머지 도움말 색은 유지합니다. Main의 첫 MENU 토큰만 Red입니다.
문자 위치는 실제 font metrics를 사용하며 EDIT의 기존 확정/이동 semantics는 같습니다.

Graph 기본 F6 INIT는 V-WIN F1 INIT와 같은 factory 창을 복구하며, EXIT가 Solver Parameters로 돌아갑니다.
TRACE는 별도 BACK 없이 EXIT로 닫습니다. ZOOM/G-Solve의 기존 bar를 유지하며, 하위 메뉴를 EXIT로 닫으면 Graph INIT가 복원됩니다.
Main 하단은 `MENU: return to MAIN MENU`이며 MENU의 실제 계산기 Main Menu 복귀 동작은 동일합니다.
EXIT 순서는 Graph → Parameters → IC → Equation → subtype/order → Main입니다.
2nd 및 Main에서 RCL로 연 Equation은 Main으로 돌아갑니다. Main의 EXIT는 Main에 머뭅니다.
계산기 메뉴에는 **MENU**로 이동합니다. 같은 실행으로 복귀하면 현재 입력을 유지합니다.
진짜 새 실행은 defaults로 시작하며 저장 파일을 자동으로 읽지 않습니다.

Main의 숫자 **1/2/3/4/5/6**은 각각 1st/2nd/N-th/SYS/RCL/SAVE입니다. F1~F5는 비어 있습니다.
Main은 **2열×3행**: 1st/2nd, N-th/SYSTEM, RECALL/SAVE입니다. 위 네 타일은 184×58, 마지막 그림 없는 행은 184×25입니다. subtype은 같은 네 타일의 **2열×2행**이며 아래 공간은 비웁니다. LEFT/RIGHT는 같은 행, UP/DOWN은 같은 열에서 순환합니다. 선택 항목은 EXE 또는 **F6 OPEN**으로 엽니다. 숫자 shortcut도 같은 handler를 사용합니다.
진짜 새 실행은 첫 항목, 내부 복귀는 기존 selector를 유지합니다. Main 하단은
`MENU: return to MAIN MENU`이며 첫 MENU는 Red입니다.
각 타일의 숫자는 우측 상단 배지에 한 번만 표시합니다. 청록/파랑 선택 테두리만 바뀌며 그래프 색은 유지합니다. 제목줄·도움말·기능키 위치는 기존 native 크기입니다.
일반 선택 메뉴는 UP/DOWN, EXE 또는 F6 OPEN, 숫자 shortcut을 지원합니다.
N-th/SYS 개수는 1~9 한 자리 정수를 입력한 뒤 첫 EXE로 확정하고 **두 번째 EXE 또는 F6 OPEN**으로 엽니다.
EXIT 편집 확정은 화면에 머뭅니다. 잘못된 값은 오류를 표시합니다.
새 기본 문서의 첫 크기 지정과 같은 크기 재진입은 확인창이 없습니다. 실제 식·IC 등의
사용자 데이터가 있는 문서의 크기 변경은 Change Equation Size를 표시합니다.
**F5 NO 또는 EXIT**는 기존 데이터를 유지하고, **F6 YES 또는 EXE**는 기본 식·IC로 교체합니다. 확인창을 연 HOLD는 무시합니다.

계산 중 EXIT 또는 AC로 중단 요청을 보낼 수 있습니다. 마지막으로 수락된 점까지의 결과가 남으며 중단 상태가 표시됩니다.

## 식과 숫자 입력

세 단계 편집 중 좌/우는 커서를 이동하고 **DEL**은 왼쪽 문자를, **AC**는 전체를 지웁니다.
Equation의 F3 VAR/F2 FUNC는 현재 식 화면을 유지하고 **하단 bar만 교체**합니다.
메뉴 열기·페이지 전환은 draft·커서·선택을 보존합니다. F1~F5로 token을 현재 위치에
삽입하면 기본 bar로 돌아옵니다. **FUNC는 실제 EDIT 상태에서만 표시**하며 SELECT의 빈 F2는 무효입니다.
커서 blink의 ON/OFF와는 무관합니다. **FUNC/VAR에서 첫 EXIT는 submenu만 닫고** draft·커서·선택·EDIT를 유지합니다.
미완성 `sinh(`도 이때 검증하지 않습니다. 다음 EXIT는 미완성 draft도 확정하고 머물며, 그 다음 EXIT가 검증 없이 부모로 이동합니다.
길게 누른 EXIT 반복 이벤트는 이 계층을 연속으로 닫지 않습니다. EXE/방향키는 메뉴를 닫고 기존 편집 동작을 수행하며,
F6는 submenu 페이지 전환만 수행합니다. VAR는 N-th의 2~9차 도함수 별칭과 SYS의 y1~y9에만 표시합니다.
모든 1차 모드·선형 2차·N-th 1차에서는 F1이 비어 있으며 눌러도 팝업이 열리지 않습니다.
V-Window 편집의 CLEAR/DEL 등은 표시된 기능을 따릅니다. TRACE의 직접 x= 입력은 제거되었으며 F1은 cursor INIT입니다.

X-CAL/Y-CAL은 **매번 빈 입력·깜빡이는 커서**로 시작합니다. 숫자, 소수점, 음수와 EXP의 과학적 표기를 입력한 뒤
**EXE 한 번**으로 검증과 계산을 실행합니다. F1~F6는 비어 있고 F6는 동작하지 않습니다.
빈 값/잘못된 값에서 EXE는 `Invalid number`와 draft를 유지합니다.
**EXIT는 값 검증 없이 draft와 현재 operation을 취소하여 G-Solve 2쪽으로 즉시 돌아갑니다.**
빈 입력, `-`, `1e`, 유효하지만 미확정인 `2.5`, 오류 표시 중에도 동일합니다.
이전 곡선 선택 화면으로 돌아가지 않으며 Graph/창/마지막 계산은 유지합니다.
누른 채 유지한 EXIT는 2쪽까지만 이동하고, 떼었다가 다시 누르면 Graph로 돌아갑니다.
다른 Equation/IC/Parameters/V-WIN 편집의 EXIT 규칙은 그대로입니다.
식당 최대 191자이며 긴 식은 선택 행 안에서 커서 주변이 보이도록 가로로 이동합니다.

| 입력 | 방법/의미 |
|---|---|
| x | ALPHA + 물리 ADD(+) 또는 기존 X,θ,T 키 |
| y | ALPHA + subtraction(SUB) 물리 키 |
| unary − | 맨 아래 행의 `(-)`(NEG) 물리 키; ALPHA와 함께 눌러도 `y`가 되지 않음 |
| y1~y9 | VAR bar의 F1~F5, F6로 페이지 전환. 실제 모드 변수만 표시 |
| sin/cos/tan | 해당 물리 키; 여는 괄호까지 삽입 |
| asin/acos/atan | SHIFT+sin/cos/tan 물리 키; FUNC에는 없음 |
| exp | SHIFT+ln |
| sqrt | SHIFT+x² |
| ln/log | 자연로그/상용로그; 항상 괄호 필요 |
| pi | SHIFT+×10^x |
| abs 및 쌍곡선 함수 | FUNC 메뉴: abs, sinh/cosh/tanh, asinh/acosh/atanh |
| e | ×10^x 키가 parser의 `e`를 삽입 |
| 과학적 표기 | EXP 키가 `e` 삽입: `1e-3` |
| { / } / , | SHIFT+물리 × / SHIFT+물리 ÷ / 물리 comma 키 |

FUNC는 위 표의 두 페이지에 abs·쌍곡선·역쌍곡선 7개만 제공합니다. VAR의 N-th는
y1~y(N−1), SYS는 y1~yN이며, 일반 1차와 Separable의 g 행은 기존 y1 alias를 제공합니다.
x 전용 계수 행은 상태 변수가 없습니다. 물리 키의 x/y는 중복 추가하지 않습니다.

곱셈은 `2*x`, `3*y`, `sin(x)*y`처럼 명시합니다. `2x`, `xy`, `sin x`는 허용하지 않습니다. `^`는 오른쪽 결합이며 `-2^2=-4`, `2^3^2=512`입니다. 음수는 `-`로 표현하며 나눗셈은 `/`입니다. 각도는 OS 설정과 무관하게 radian입니다. 숫자 폼도 상수식 `pi`, `1/4`, `sqrt(2)`를 허용하지만 x/y 상태 변수는 허용하지 않습니다.

식의 변수 범위가 맞지 않거나 괄호가 빠지면 NEXT에서 식 이름과 문자 위치를 보여줍니다. 오류 draft를 수정하고 다시 NEXT 하세요. Parameters GRAPH에서 전체 문서를 다시 검증합니다. 숫자 단계의 OPTN은 함수 삽입 메뉴입니다. 내장 pi/e는 유지되며 A~Z/r/theta 개인 상수는 새 식에서 사용할 수 없습니다.

## 방정식 모드

| 메뉴 | 화면에 입력하는 함수 | 주의 |
|---|---|---|
| 1st → Separable | y'=f(x)g(y)의 f, g | f에는 x, g에는 y만 사용 |
| 1st → Linear | y'+f(x)y=g(x)의 f, g | 두 함수 모두 x와 상수만 사용 |
| 1st → Bernoulli | y'+f(x)y=g(x)y^n의 f, g, n | n은 수치 입력 |
| 1st → Others | y'=f(x,y)의 RHS | 일반 1차 |
| 2nd | y''+f(x)y'+g(x)y=h(x)의 f, g, h | 선형 2차; 세 함수는 x와 상수만 사용 |
| N-th | y^(N)=F(x,y,y1,...,y(N−1))의 RHS | 차수 1~9, y1은 y', y2는 y'' |
| SYS | 각 yj'=Fj(x,y1,...,yN)의 RHS | 크기 1~9, y1은 첫 상태, y2는 둘째 상태 |

2차 비선형 문제는 **N-th → 2**를 사용합니다. N-th의 `y`는 SYS로 변환하면 `y1`, N-th의 `y1`은 SYS의 `y2`가 됩니다. N-th 화면의 OPTN → Convert to system는 식의 토큰과 IC를 함께 변환합니다. 단순 문자열 치환으로 `sin` 같은 이름을 훼손하지 않습니다.

같은 종류·차수를 다시 선택하면 현재 작업을 유지합니다. 다른 종류/차수는 해당 모드의 예제 식과 IC로 바뀌지만 V-Window, Solver, Grid/Axis Label과 Field Density/Style/Color는 app 전체 설정으로 유지됩니다. 1st editor의 EXIT는 subtype 목록, subtype EXIT는 Main으로 돌아갑니다. N-th/SYS editor의 EXIT는 차수 숫자 입력 화면으로 돌아갑니다.

## 초기조건·설정·출력

방정식 화면은 식/계수만, 다음 Initial Conditions 화면은 초기값만 표시합니다.
**1차 네 모드**는 공통 `x0`와 `y0` 두 행입니다. `y0=0`, `{1}`, `{0,1}`, `{0,1,-1}`을
입력할 수 있습니다. 예를 들어 `x0=0, y0={0,1}`은 (0,0), (0,1)에서 출발하는 별도 두 해입니다.
중괄호는 SHIFT+× / SHIFT+÷, 구분자는 물리 comma 키입니다. 각 항목은 `1/4`, `sqrt(2)`,
`pi` 같은 숫자식이며 최대 **10개 / 전체 191자**입니다. 빈 목록, 비대칭 괄호, 빈 항목,
잘못된 변수, undefined/NaN/Inf, 절댓값 1e100 초과를 거부합니다. 실패하면 기존 값과 draft를 보존합니다.
중복 값은 허용하며 별도 해로 계산하므로 같은 곡선이 겹칠 수 있습니다. 저장되는 것은 계산된 숫자값이며
목록은 그 값으로 다시 표시합니다. x0를 바꾸면 모든 해의 공통 시작 x가 바뀝니다.
화면에는 `y0: scalar or {values}; at most 10` 설명과 SELECT 상태의 `Comma: separator`를
표시합니다. 중괄호 shortcut의 화면 안내는 제거했으며 물리 키 입력은 그대로 지원합니다.
EDIT 상태의 하단은 공통 확정 안내로 바뀝니다. 11번째 값은 `Too many initial values / Max: 10`,
191자를 넘는 입력 시도는 `Input too long / Max: 191 characters`로 구분합니다.
초과 문자는 버퍼에 쓰지 않으며 길이 안내가 표시되면 DEL/AC로 수정한 후 확정합니다.
`{0,1,2,3,4,5,6,7,8,9}`의 10개 해 모두 Graph/TRACE/G-Solve와 Table/STAT에 포함되며,
Table은 x를 고정한 채 좌우로 y1~y10 열을 이동합니다. 기존 총 계산량 한도는 늘리지 않습니다.

**2차는 x0/y0/y'0**, **N-th는 x0와 n개 state 초기값**, **SYS는 x0와 m개 state 초기값**을
입력합니다. 이 여러 숫자는 **한 해를 정의하는 하나의 완전한 초기 벡터**입니다. 독립적인 여러
IC set을 뜻하지 않습니다. 고차/SYS의 새 UI는 한 벡터만 받으며, 상태별 scalar에 목록을 넣지 않습니다.
전체 원본 매뉴얼의 근거와 일반 Output matrix의 해석 한계는 [IC audit](IC_BEHAVIOR_AUDIT.md)에 있습니다.
모든 IC 화면에 F1 INIT가 있고 F2~F5는 비어 있습니다. F6 NEXT와 EXIT를 사용합니다.

**Solver Parameters**의 x min/max는 적분 구간입니다. 처음에는 `ceil(V-Window Xmin)`과
`floor(V-Window Xmax)`이며 사용자가 V-Window/Graph pan/zoom을 조작하면 따라갑니다. TRACE runtime 추적은 이 자동 범위도 변경하지 않습니다. 직접 편집한 범위는 user override로
유지됩니다. 범위 전체의 기존 상태를 두 행의 **AUTO/MAN**으로 표시합니다. 직접 확정하면 값이 같아도 MAN이며, INIT는 AUTO를 복구합니다. 편집 중인 행의 상태 표시는 잠시 숨깁니다. **SF(0~50, 기본값 12)는 scalar 1차 네 모드에서만 Parameters에 표시합니다.**
Method는 세 번째 행이며 LEFT/RIGHT로 RK4/RK45를 전환합니다. 계산은 GRAPH/EXE에서 시작합니다.
RK4는 h → Step → (1차 SF) → Max steps, RK45는 h0 → RelTol → AbsTol → (1차 SF) → Max steps입니다.
2nd/N-th/SYS는 차수·변수 개수 1을 포함하여 SF 행이 없습니다. RK45의 숨겨진 Step은 사용하지 않습니다.
UP/DOWN, 편집 EXE의 다음 행, 선택 행 도움말도 표시되는 행만 따릅니다.
Parameter F1 INIT는 Method를 유지하고 자동 범위 추종, h=.1, Max Steps=20000을 복구합니다.
RK4에서는 Step=1, RK45에서는 RelTol=1e-6/AbsTol=1e-9로 복구하며 다른 방식의 숨겨진 설정은 보존합니다.
1차에서는 SF도 12로 복구하며, 고차/SYS에서는 숨겨진 SF 값을 보존합니다.
예를 들어 SF=20 → 2nd → INIT → 1st에서도 20입니다. SAVE/RCL도 숨겨진 SF를 보존하며
새 저장은 v11이며 기존 v3~v10 세션의 읽기 호환성을 유지합니다. Field Style/Color는 모든 모드에서
보존합니다. Solver 수정은 V-Window를 역으로 변경하지 않습니다.
Xdot은 `(Xmax-Xmin)/378`이며 Xdot 편집은 Xmax를 변경합니다.

**OUTPUT은 각 출력 곡선의 표시 여부와 색을 설정합니다.** 1차 IC가 하나면 `y`,
여러 개면 `IC1 y`~`IC10 y`를 표시합니다. 각 행에서 LEFT/RIGHT는 그 IC만 ON/OFF,
F3 COLOR는 그 IC만 색을 바꿉니다. UP/DOWN은 선택 행 이동/순환이며 7행씩 표시하여 IC10까지 접근합니다.

예: `IC1 y ON`, `IC2 y OFF`, `IC3 y ON`, `IC4 y OFF`, `IC5 y ON`.
Graph에는 IC1/3/5만 그리며 TRACE/G-Solve도 이 곡선들만 선택합니다. 하나만 ON이면
G-Solve의 곡선 선택을 생략하고 ICPT는 ON 곡선이 둘 이상일 때만 가능합니다.
Table/CSV/STAT은 항상 x를 첫 열로 두고 ON IC만 포함합니다. 여러 IC의 기존
machine-safe 열 이름은 y1~y10이며 위 예시는 x,y1,y3,y5입니다. 숨긴 열 때문에 번호를 바꾸지 않습니다.
모두 OFF이면 axes/grid와 설정된 SF는 유지하고 곡선은 없습니다. TRACE는 No visible graph,
G-Solve는 Not available, Table/CSV/STAT은 x-only입니다. OFF IC도 내부 적분·Event 검출과
실제 작업량 통계에는 포함되며 해당 Graph Event marker만 숨깁니다.

2차는 y/y', N-th는 각 도함수, SYS는 y1~ym의 기존 종속변수 ON/OFF와 색을 유지합니다.
SYS2 PHASE는 기존처럼 Output와 별개로 두 state를 사용합니다. x 행은 없습니다.
OFF 행도 실제 색의 선 미리보기를 유지합니다. 색 선택은 F3에서 방향키, EXE 적용 / EXIT 취소입니다.
기본 색은 Magenta → Cyan → Bright Green → Red → Blue → Black, 7번째부터 반복합니다.
IC 수 감소 후 재확장은 숨겨진 slot의 ON/OFF와 색을 복원하며, 아직 편집하지 않은 새 slot은
ON과 기본 색입니다. IC 값 수정·IC INIT는 Output을 초기화하지 않습니다.
**Output F1 INIT**는 전체 10개 slot을 ON·기본 palette·첫 선택 행으로 복구합니다.
SAVE v11/RCL은 비활성 slot과 all-OFF도 보존합니다. 구버전 v3~v10은 모든 IC를 ON으로
불러오며 기존 색을 보존합니다. 과거 공통 y가 OFF였어도 새 IC bits는 ON으로 초기화합니다.
[상세 소비자·저장 정책](VISIBILITY_PROMPT_AUDIT.md)을 참고하세요.
OUTPUT EXE는 다음 행으로 이동하며 마지막 행은 수정했으면 한 번 확정/머묾, 다음 EXE가 DONE입니다.
수정하지 않은 마지막 행은 곧바로 DONE입니다. F1 INIT는 모든 종속변수 ON과 기본 solution 색,
첫 선택 행을 복구하며 SF/field 외형은 유지합니다.

V-WIN INIT, Parameter INIT, OUTPUT INIT, Graph Settings INIT는 값을 복구하고 화면에
머물며 첫 항목 선택·편집 종료 상태가 됩니다. Parameters F5 SET은 Graph Settings로 직접 들어가고
EXIT는 정확한 이전 Parameters 선택 행으로 돌아갑니다.

INIT 범위는 분리됩니다. Parameters는 선택 Method와 숨겨진 방식 설정을 보존하며 numerical 설정만,
V-WIN은 좌표 범위/scale/Xdot만, Graph Settings는 Grid/Label/field 외형만, Output은 출력/색만 초기화합니다.
V-WIN INIT는 Grid/Label 및 Phase 투영 선택을 보존합니다. Solver의 기존 AUTO 범위 추종은 유지되므로
TIME 창이 변하면 자동 Xrange가 따라갑니다. 수동 Xrange/h/tolerance/Event는 V-WIN INIT로 초기화하지 않습니다.

## 기울기장 — 밀도는 Parameters, 외형은 Graph Settings

**SF=0은 OFF, 1~50은 화면 열 수**입니다. 세로 행 수는 화면 비율에 맞추며 SF12는 12×7,
SF50은 50×26점입니다. 50을 넘는 입력은 필드에 오류를 표시하며, 이전 session에 저장된 50 초과 값은 load 시 50으로 제한합니다. 저장 형식은 유지합니다. SF는 h/Step/적분 구간/IC/TRACE 이동 간격을 바꾸지 않습니다.

Graph Settings는 Grid / Axis Label와 간격을 둔 Slope Field heading 아래 Style / Color만 둡니다.
Grid/Axis Label은 LEFT/RIGHT로 토글하고 F1 INIT로 초기화합니다. EXE의 완료 동작은 동일합니다.
Style은 LEFT/RIGHT로 변경하며 F1 INIT는 이 행에서도 항상 보입니다. F2는 비어 있습니다.
Style 도움말은 `LEFT/RIGHT: SEGMENT/ARROW toggle`, Color 도움말은 `RIGHT/F3: COLOR`입니다.
Color chooser는 RIGHT 또는 F3 COLOR로 엽니다. 기본은 **Arrow / Pale Blue**입니다.
Color 행은 이름 옆에 실제 선택색의 직사각형 견본을 표시합니다. OUTPUT과 같은 테두리·크기를
사용하며 Pale Blue/Red/Cyan/Magenta/Gold/Gray의 기존 색상표와 선택·취소 동작을 유지합니다.
Graph Settings F1 INIT는 Grid/Axis Label ON, Arrow/Pale Blue를 복구하고 **SF는 바꾸지 않습니다**.
사용자가 v5 세션에 명시적으로 저장한 Segment와 field 색은 그대로 복원합니다.

1st의 네 scalar 모드만 field를 표시합니다. 2nd/N-th/SYS(차수·변수 개수 1 포함)는 SF와 외형 항목을 숨기고
전역 선호값을 유지합니다. field는 여러 IC에서도 한 번만 그립니다. OUTPUT y OFF와 SF ON으로
field만 표시할 수 있습니다. 새 IC 화면은 빈 목록을 허용하지 않습니다.

Segment는 짧은 막대, 기본 Arrow는 증가하는 x의 접선 `(1,F(x,y))` 방향입니다.
F=0이면 오른쪽 수평, 양수면 오른쪽 위, 음수면 오른쪽 아래입니다. X/Y scale과 셀 크기로
정규화하고 정의되지 않은 grid point는 건너뜁니다. solution/TRACE보다 먼저 그리는 얇은 배경이며
전체 field 배열을 보관하지 않습니다.

Field 팔레트는 기존 선명한 solution 팔레트와 별개입니다. 설치된 gint `C_RGB`의 각 입력은 0~31입니다.
아래 값은 불투명 RGB565이며 실시간 alpha-blending은 없습니다.

| 색 | gint 입력 | 실제 RGB565 |
|---|---|---|
| Pale Blue | `C_RGB(17,21,27)` | `0x8d5b` |
| Pale Red | `C_RGB(27,17,17)` | `0xdc51` |
| Pale Cyan | `C_RGB(12,23,24)` | `0x65d8` |
| Pale Magenta | `C_RGB(25,17,25)` | `0xcc59` |
| Pale Gold | `C_RGB(24,21,9)` | `0xc549` |
| Gray | `C_RGB(18,18,18)` | `0x9492` |

흰 배경에서의 host 렌더링은 확인했습니다. 실제 LCD의 색 가독성·최대 밀도의 화살촉·blink 복원은
**HARDWARE RETEST REQUIRED**입니다.

## 그래프·Trace·phase

컬러 곡선은 양방향으로 적분한 IC family를 약 2 px 두께로 표시합니다. 색은 OUTPUT의 상태별 설정과 IC 순환 palette를 따릅니다. factory Grid와 Axis Label은 ON이며 저장된 OFF 값은 유지됩니다. 축·grid·slope field의 두께는 그대로입니다. 화면 밖 선분은 clipping하지만 적분은 계속합니다.

- 방향키: 표시 창을 폭/높이의 20%만큼 pan.
- `+`/`−` 또는 ZOOM: 중앙 기준 확대/축소. TIME에서 Solver range를 직접 override하지 않은 상태에서는 새 X window에 맞춰 적분 구간도 갱신됩니다.
- ZOOM은 현재 그래프 위에서 F1 IN / F2 OUT / F3 AUTO / **F4 ORIG / F5 BOX**를 표시합니다. 각 동작 후에도 ZOOM bar를 유지하며 **EXIT로만 닫습니다**. ZOOM 안에서도 방향키로 pan하며 메뉴가 유지됩니다. 메뉴를 열고 닫기만 하면 재적분·파일 I/O가 없습니다.
- ORIG는 V-WIN INIT와 같은 기본 창으로 돌아갑니다. 식·IC·h는 유지하며 TIME의 Solver AUTO만 새 창을 따릅니다. 수동 Solver 범위는 유지됩니다.
- AUTO는 현재 X 범위를 유지하고, Solver와 X 창의 교집합에 있는 ON 상태의 곡선의 유한한 값으로 Y 범위를 맞춥니다. 현재 Y 창 밖의 값도 사용하며 상수 곡선에도 여백을 줍니다. 유효한 점이 없거나 계산이 중단되면 기존 창을 보존하고 좌측 최상단에 안내합니다. SYS 2D PHASE AUTO는 보관된 원래 trajectory의 y1/y2 전체 유한 범위에 여백을 더해 두 축을 맞춥니다. TIME AUTO는 현재 X 창을 유지합니다.
- OPTN 물리 키: SYS 2D에서는 Graph Settings와 계산 상세입니다. 다른 2-state 이상 모드는 기존 phase projection/축 선택을 유지합니다.
- TRACE는 진입 당시 **가로 창과 Xscale/Xdot을 고정**합니다. 일반 Graph의 X/Y pan과 구간 확장은 그대로 유지합니다.
  TIME에서는 고정 X 창과 선택한 해의 연결된 유효 수치 구간의 교집합만 탐색합니다. 수동 Solver 범위가 창보다 넓거나 좁아도 경계에서 멈추며, TRACE 때문에 캐시를 확장하지 않습니다.
  **F1 INIT / F2 NORMAL / F3 FAST / F4 FASTER / F5 LEFT / F6 RIGHT**입니다.
  INIT는 노랑/검정, NORMAL은 주황/검정, FAST는 Bright Green/검정, FASTER는 Cyan/검정입니다.
  NORMAL/FAST/FASTER는 **1×/2×/3× 진입 시 실제 Xdot** 이동 간격이며 키 반복 속도 변경이 아닙니다. 선택 속도 테두리는 유지합니다.
  F1 INIT는 이번 TRACE의 최초 cursor와 curve로 복귀하고 현재 속도·solver·식·IC·Event·Output을 보존합니다. 직접 숫자 x= 입력은 제거했습니다. 특정 x의 수치 평가에는 G-Solve Y-CAL을 사용합니다.
  F5/F6는 설정된 Solver Xrange min/max를 목표로 하되, 화면 밖 또는 Event/invalid 경계 너머이면 현재 연결된 유효 구간의 가장 가까운 경계에 멈춥니다. FASTER도 overshoot하지 않습니다.
  UP/DOWN은 curve를 바꾸며 가능한 경우 x를 유지합니다. 유효한 y가 상하 여백을 넘으면 **Y만 평행 이동**하여 커서를 다시 표시합니다. 기존 10% trigger / 30% landing margin을 사용하며 Y span/Yscale, X bounds/Xscale/Xdot, h/Step과 Solver 설정을 유지합니다.
  NaN/Inf·magnitude-invalid 값, 부동소수점 해상도 때문에 span이 유지되지 않는 큰 이동은 거부합니다.
  보관점 사이의 유효한 linked 구간을 선형 보간하므로 표시 y는 화면용 근사입니다. gap이나 Event STOP을 가로질러 가짜 값을 만들지 않습니다.
  SYS2 PHASE에서는 가로 y1 범위를 고정하고 세로 y2만 따라갑니다. 이동 순서는 적분 x이며 TIME 창의 Xdot을 사용합니다. 다음 보관 상태가 가로 범위를 벗어나면 멈추고 이후 재진입 구간으로 건너뛰지 않습니다. 아주 좁은 창에 보관 상태가 없으면 해당 창에서 TRACE unavailable을 표시하고 그래프를 유지합니다.
  TRACE 종료 후 일반 Graph/ZOOM/G-Solve-menu 방향키 pan은 기존 안전한 재계산과 AUTO 범위 갱신을 수행합니다. HOLD 중 EXIT/MENU 우선 처리와 2px 선택 곡선 blink, 9px point cross는 유지합니다.

Graph numerical/domain 경고는 공통 **좌측 상단 (7,4)**에 normal-weight 빨간 글씨로 표시합니다. 뒤에는 실제 문자열 폭·높이와 작은 여백만큼 불투명한 흰 배경을 그립니다. 그래프 높이는 줄이지 않으며, 문구가 바뀌거나 없어지면 기존 redraw가 복구합니다. Event STOP은 정상적인 중립색 `END: Event`입니다. 우측 TIME/PHASE/EVT와 두 번째 줄 Phase legend를 분리합니다. 곡선 선택 안내도 같은 좌측 최상단 채널을 쓰며 활성 동안 경고를 잠시 숨겼다가 종료 즉시 복구합니다. 경고는 비치명 상태이며 유효 구간 TRACE/G-Solve는 계속 사용할 수 있습니다.

G-SLV 메뉴를 열거나 페이지를 바꿨다가 EXIT만 하면 그래프와 계산 횟수가 유지됩니다. G-SLV의 수치 범위는 현재 V-Window X 범위와 Solver 범위의 교집합입니다. Ymin/Ymax는 그리기와 pointer clipping에만 사용됩니다. ROOT, MAX/MIN, Y-ICPT(x=0), Y-CAL, X-CAL과 두 보이는 곡선의 교점 ICPT는 화면 밖 y도 계산하고 좌표를 하단에 표시합니다. G-Solve menu에서 방향키는 pan이고, operation을 고른 뒤 현재 candidate가 깜빡이며 UP/DOWN은 blink 대상을 옮깁니다. EXE는 곡선을 확정하고 LEFT/RIGHT는 여러 결과를 이동합니다. 보이는 곡선이 정확히 둘이면 ICPT가 자동 선택하며 더 많으면 두 graph를 순서대로 고릅니다. 결과 없음과 오류는 dialog 없이 같은 graph 하단에 표시됩니다. 접하는 근, h 사이의 매우 빠른 진동, 특이점과 불완전 trajectory는 놓칠 수 있습니다.

G-Solve 선택 안내는 모든 연산에서 **UP/DOWN: SELECT GRAPH, EXE: SELECT**입니다.
EXE만 파란 normal 글자로 한 번 표시합니다. 선택·결과에서 **EXIT 한 번 → G-Solve submenu**,
그 다음 새 EXIT → Graph입니다. HOLD는 여러 계층을 통과하지 않습니다. 선택 취소나 scratch
계산 취소는 원래 그래프·수치 진단을 보존하며 `Partial: Cancelled`를 만들지 않습니다.
Drawing 취소는 새 결과를 확정하지 않습니다. 최초 계산은 Parameters로 돌아가고, 재계산은 이전 창·수치 보고서와 보관된 그래프를 복원합니다.

결과 cross가 화면 위/아래 또는 고정 하단 패널에 가려지면 **Y만 필요한 만큼 평행 이동**합니다.
이미 안전하게 보이면 창을 움직이지 않습니다. X 범위·scale, Y span·scale, solver 설정과
수치 결과·선택 index는 그대로입니다. LEFT/RIGHT 결과 순회에서도 기존 결과와 보관된 표시
표본을 사용하며 G-Solve를 다시 계산하지 않습니다. invalid 결과를 따라 창을 움직이지 않습니다.

Graph **F6 INIT**, V-WIN **F1 INIT**, ZOOM **F4 ORIG**는 같은 factory 창 초기화를 공유합니다.
현재 TIME/PHASE와 Grid/Label을 유지하고 식·IC·Method·h/h0·허용오차·Event·Output·색·저장을
초기화하지 않습니다. TIME AUTO 범위만 기존 정책대로 창을 따르고 MAN 범위는 유지합니다.
예전 Graph entry snapshot은 더 이상 사용하지 않습니다. **TRACE F1 INIT**는 별도 커서 복귀입니다.

오래 걸리는 **Table** 준비는 파란 `Preparing Table... /` 제목, 흰 `EXIT cancels`
행과 본문이 있는 별도 화면이며 F1~F6 strip을 숨깁니다.
**Drawing은 현재 Graph를 유지**하고 하단 F-key strip만 하나의 파란 바로 바꿉니다.
왼쪽 흰 `Drawing... /`, 오른쪽 흰 `EXIT cancels`이며 버튼 구분선은 없습니다.
둘 다 약156ms 후 나타나고 `/ - \ |`를 최대8Hz로 갱신합니다. Drawing은 bar만 갱신합니다.
EXIT를 먼저 확인하고 취소 시 임시 계산을 버려 안정된 결과와 원래 F-key로 복구합니다.
완료 시에도 Graph와 F-key를 복구하며, 최초 Graph 진입은 axes 화면을 한 번 만든 뒤 계산합니다.
Table의 같은 페이지에서 열만 이동하거나 동일 TOP/BTM/MID를 누르면 재계산·대기 화면이 없습니다.
TRACE와 G-Solve는 기존 하단 `CALCULATING... /`를 유지합니다. 빠른 작업은 대기 화면을 생략합니다.
Graph 취소 복원은 기존 제한된 표시 표본을 사용하므로 원래 고밀도 그림과 일부 픽셀 차이는 가능합니다.
전체 invalid x 영역을 붉게 칠하지 않습니다. 현재 per-curve 상태로 전체 영역의 수학적 부재를
단정할 수 없으므로 빨간 경고와 invalid segment 생략을 유지합니다.

### SYS 2D Phase Portrait와 분석

**SYS → 2 → 식 → IC → Parameter → GRAPH → F4 VIEW → F2 PHASE**.
가로는 y1, 세로는 y2입니다. F4 VIEW에서 F1 TIME 또는 F3 TABLE로 이동합니다.
Table은 항상 독립변수 x와 종속변수의 time-domain 표입니다. SYS IC는 기존과 같이 완전 벡터 하나입니다.

TIME과 PHASE의 V-Window는 별도로 보존됩니다. Phase V-WIN, 방향키 pan, ZOOM IN/OUT/AUTO/ORIG는
TIME 창·식·IC·h·Step·Solver Xrange를 바꾸지 않습니다. 첫 Phase 창은 원래 수치 스트림에서 얻은
유한 y1/y2 범위에 12% 여백을 더합니다. 유효점이 부족하면 두 축 [-3.1,3.1]을 사용합니다.
ORIG도 이 기본 창으로 돌아갑니다. 가로/세로 pixel scale이 다르면 원 궤적이 타원처럼 보입니다.

Phase는 처음 TIME을 그릴 때 함께 보관한 기존 258점 TRACE 캐시를 projection합니다.
Phase를 위한 별도 적분이나 trajectory 복제는 없습니다. 최초 TIME은 전체 스트림을 그리며,
이후 캐시 재표시와 TRACE는 긴 구간·빠른 진동에서 해상도가 낮아질 수 있습니다.
Phase AUTO의 범위는 decimation 전 전체 스트림에서 얻습니다. invalid gap은 연결하지 않습니다.
Output OFF는 TIME/Table 표시를 제어하며 Phase에 필요한 y1/y2 계산과 projection은 유지됩니다.

PHASE F5 **ANLYS**는 그래프 위 한 단계 메뉴입니다. EXIT로 기본 Graph bar에 돌아옵니다.

- **F1 FIELD**: 20×11의 연한 파랑 화살표를 ON/OFF. (f1,f2)를 창 span과 pixel 비율에 맞춰
  정규화합니다. 정확한 zero vector는 작은 점이며 정의역 오류가 있는 위치는 건너뜁니다.
- **F2 NULL**: 두 nullcline을 함께 ON/OFF. 빨강 **N1: f1=0**, 파랑 **N2: f2=0**.
  20×20 cell의 수치 contour이며 symbolic 식이 아닙니다. FIELD와 별도로 켜고 끕니다.
- **F3 EQPT**: 현재 Phase 창 안에서 coarse grid 후보를 찾고 bounded Newton으로 정제합니다.
  최대 16점, 중복 제거, 잘 보이는 diamond marker를 사용합니다. LEFT/RIGHT로 결과를 순회하며
  아래에 y1/y2와 `Linearized:` 분류를 표시합니다. `+`는 결과 제한 도달입니다.
  찾지 못했다고 평형점이 없다고 증명한 것은 아닙니다.
- **F4 INFO**: 선택한 점의 numerical Jacobian과 근사 eigenvalues를 표시합니다.
  Saddle, Stable/Unstable Node, Stable/Unstable Spiral, Center/Neutral, Inconclusive,
  Unavailable은 **국소 선형화** 결과입니다. Center/Neutral 및 non-hyperbolic 결과로
  비선형·전역 안정성을 확정하지 않습니다. 미분을 신뢰할 수 없으면 Unavailable입니다.

RHS bytecode에서 실제 x instruction을 검사합니다. non-autonomous SYS도 Phase trajectory를
그릴 수 있습니다. FIELD와 NULL은 **IC x0**에서 평가하며 `Field at x=...`를 표시합니다.
이 경우 EQPT/stability는 `Autonomous systems only`로 제한합니다. 시간 애니메이션은 없습니다.

수치 분석은 작업량 제한과 EXIT/MENU poll을 사용합니다. 취소/실패한 EQPT는 이전 결과를 유지합니다.
FIELD/NULL은 먼저 화면을 바꾸지 않고 검증한 뒤 같은 bounded 연산으로 화면을 완성합니다.
마지막 atomic paint 중 들어온 키는 다음 입력에서 처리합니다. 실제 긴 식의 반응 시간은
**HARDWARE TEST REQUIRED**입니다. 유한 grid는 좁은 구조와 미검출 불연속을 모두 증명할 수 없습니다.
알고리즘·허용오차·예산은 [Phase numerical notes](PHASE_NUMERICS.md)에 있습니다.

Phase TRACE는 x, y1, y2를 표시하며 NORMAL/FAST/FASTER, LEFT/RIGHT, F5/F6 endpoint와 EXIT를
유지합니다. SYS single trajectory에서 UP/DOWN은 불필요한 곡선 전환을 하지 않습니다.
다른 모드의 기존 phase projection/축 선택은 OPTN에서 계속 사용할 수 있습니다.

### 수치 guard와 유효한 구간

`|value| > 1e100`은 application guard, NaN/Inf와 수학 domain 오류는 별도 원인입니다.
guard는 유지하며 실패한 점을 그리거나 빈 구간 양쪽을 선으로 연결하지 않습니다.
Graph의 `END: Magnitude > 1e100` 등은 계산된 유효한 구간을 폐기하지 않습니다.
명시적 Drawing 중단은 임시 계산을 버리고 이전 화면·결과를 복원합니다. 취소된 Partial 그림을 확정하지 않습니다.

TRACE는 고정한 화면 안의 연결된 유효 점에서 이동하고 경계에서 멈춥니다.
다른 유효 구간이 있어도 현재 component의 gap을 넘어 이동하지 않습니다. X-CAL/ROOT/MAX/MIN/ICPT는
계산된 유효 구간에서만 찾고 x순으로 결과를 표시하며, 빈 구간을 가로질러 근·극값을 만들지 않습니다.
Y-CAL은 요청한 x까지 해당 IC에서 적분할 수 있을 때만 값을 반환합니다. TRACE는 보관된 유효 구간의 화면용 보간을 사용합니다.

**두 solver 모두 각 IC에서 양방향으로 성공한 prefix를 보존합니다.** 실패 지점 너머의 같은 해를
새 초기조건 없이 재시작하지 않습니다. 반대 방향과 별도 IC는 계속 사용할 수 있지만 별도 IC는 별도 곡선입니다.
합성 valid/gap/valid 테스트는 신뢰 가능한 양쪽 값이 주어졌을 때의 소비 경로를 검증하며,
일반 ODE가 특이점을 통과했다는 뜻이 아닙니다. 결과 없음은 계산되지 않은 영역의 해 부재를 보장하지 않습니다.
자세한 경계는 [Numerical validity audit](NUMERICAL_VALIDITY_AUDIT.md)에 있습니다.

## Event Detection / Solver Info

Parameters **F2 ADV**에서 **F1 EVENT**, **F2 INFO**를 엽니다. 나머지 softkey는 비어 있습니다.
INIT는 Parameters F1에 있으며 numerical solver parameters만 복구하고 Event 설정은 유지합니다.
EXIT는 현재 child 화면 또는 ADV submenu만 닫습니다. 설정·INFO 방문만으로 계산하거나
파일을 쓰지 않습니다. Event는 document당 하나이며 OFF에서도 입력한 설정을 보존합니다.

Event Settings는 Enabled, E, Direction, Action 네 행입니다. LEFT/RIGHT로 선택값을 바꾸고
E 행은 기존 inline 식 편집기와 FUNC/VAR를 사용합니다. F6 DONE/선택 상태 EXE로 돌아갑니다.
기본은 OFF / 빈 E / ANY / MARK입니다. ON이면 GRAPH 전에 E를 compile하고 모든 IC에서
평가합니다. 빈 식·사용 불가 변수·IC의 domain 오류는 먼저 수정해야 합니다.

**E(x,state)=0**의 crossing을 찾습니다. 1차 `y`, 2차 `y`와 `y1`(y'), N-th `y1…y8`(도함수),
SYS `y1…yn`을 기존 parser와 동일하게 사용합니다. N-th → SYS는 Event 식도 함께 변환합니다.
ANY는 양쪽 crossing, RISING은 음→양, FALLING은 양→음입니다.
**RISING/FALLING은 backward 적분에서도 항상 x가 증가하는 방향 기준입니다.**

MARK는 계속 적분하며 최대 32개의 검정 외곽선·주황 중심 사각 marker를 Graph/Phase에 표시합니다.
그 이후 hit도 INFO에서 계속 셉니다. 여러 출력에서는 첫 표시 변수에만 marker를 둡니다.
STOP은 각 IC와 적분 방향의 첫 matching root에서 종료하며 반대 방향과 다른 IC는 계속 계산합니다.
Table의 마지막 유효 행은 root x/state이고 `END: Event`를 표시합니다. TRACE는 root에 멈추며
그 바깥으로 확장하지 않습니다. G-Solve 역시 그 이후에 해가 있다고 가정하지 않습니다.

예: Others에 `y`, IC (0,1), E=`y-10`, RISING/STOP → x≈ln(10)=2.302585093에서 종료합니다.
RK4 h=.1의 x 오차는 약1.76e-6, RK45 기본 tolerance의 직접 query 오차는 약3.61e-7입니다.
이는 해당 문제의 측정값이며 모든 식에 같은 정확도를 보장하지 않습니다.
IC의 E=0은 ANY에서 즉시 hit입니다. RISING/FALLING은 첫 유효 nonzero 진행의 방향으로
원래 IC의 hit 여부를 판정합니다. 계속 0인 directed Event는 반복 hit하지 않습니다.
초기 zero plateau의 directed STOP은 표시 전에 scratch로 방향을 판정하고 그 비용도 셉니다.

검출은 accepted numerical step의 endpoint 부호를 사용합니다. **한 step 안에서 여러 번 0을 지나
양끝 부호가 같거나 접하기만 하는 root는 놓칠 수 있습니다.** 작은 RK4 h 또는 엄격한 RK45 tolerance가
도움이 될 수 있지만 RK45는 E의 진동이 아닌 ODE의 오차를 제어하므로 모든 hit를 보장하지 않습니다.
Event 값만 domain/NaN/magnitude-invalid이면 그 구간의 Event 분석을 건너뛰고 정상 ODE는 계속합니다.
48회 이내 secant/bisection과 선택 solver의 scratch target landing으로 root를 구합니다.

**INFO는 마지막 trajectory run의 snapshot**입니다. Method, Status, 설정 범위·IC수·차원·h,
RK4 Steps, RK45 RelTol/AbsTol/Accepted/Rejected/Attempts, 실제 RHS evals, h min/max를
UP/DOWN으로 읽습니다. Event ON이면 Direction/Action/Hits/Stored/Unavailable도 보입니다.
단일 STOP은 Stopped x를 표시합니다. 여러 IC는 합계를 표시합니다.
Accepted/Steps는 refinement와 초기 방향 probe까지 포함하는 성공한 numerical trial 횟수이며,
화면에 그린 점 수와 다릅니다. RHS도 그 모든 실제 solver 호출을 포함합니다.
RK45 Rejected는 오차 제어의 정상적인 일부일 수 있고 Attempts=Accepted+Rejected입니다.
취소·실패한 trial은 Rejected에 포함될 수 있으며 완전한 trial보다 RHS 호출 횟수가 적을 수 있습니다.
h min/max도 scratch를 포함한 수락 step의 절댓값입니다. Phase FIELD/NULL/EQPT 작업은 제외합니다.

새 Graph 계산과 성공한 TRACE 준비는 보고서를 갱신합니다. Graph 실패/취소도 실제 작업량과
상태를 남깁니다. 취소한 TRACE 준비는 이전 graph·marker·보고서를 함께 보존합니다.
Table/G-Solve/Phase 분석과 INFO 방문은 보고서를 덮어쓰지 않습니다.
새 문서·load·recall 후에는 `No solver run yet`이며 Event 설정만 SAVE v11에 저장됩니다.
[정확한 수치 정책과 한계](EVENTS.md), [하드웨어 재시험](HARDWARE_RETEST.md)을 참고하십시오.

## Solver Methods: RK4와 RK45

기존 계산 결과와 고정 h 비교에는 기본 RK4를 사용합니다. 구간마다 변화율이 크게 달라
자동 step 조절이 필요하면 RK45를 선택합니다. RK45는 Dormand–Prince 5(4)로
component별 AbsTol+RelTol×상태 크기에 대해 최대 local error norm ≤1인 단계만 수락합니다.
RelTol은 상대 오차 목표, AbsTol은 작은 값의 절대 오차 바닥입니다. 기본값은 1e-6/1e-9이며
기존 numeric editor의 EXP와 NEG로 입력합니다. 0/음수/NaN/Inf는 거부하고
RelTol<100×machine epsilon, AbsTol<최소 normal double은 Tolerance too small입니다.

같은 h 값을 공유하므로 방식 전환 시 보존됩니다. RK45 h0=.1은 첫 제안이며
실제 내부 h는 자동으로 변합니다. RelTol/AbsTol과 숨겨진 RK4 Step도 방식 전환·RAM Recall·새 식에서 유지됩니다.
RK45 Max steps는 IC/방향 또는 수치 query당 수락+거절 시도 합계(기본20000, 상한100000)입니다.
전체 동작에서 최대200000시도/800000 RHS/2400000 RHS×차원을 runtime에 제한합니다.
STAT 전체 export도 같은 budget을 공유합니다. EXIT는 거절 재시도·각 stage에서도 검사합니다.

RK45 Step은 숨기며 Graph/Table 출력은 TIME Xdot과 범위/1024 중 큰 간격의 IC 기준 격자입니다.
수치 Table/G-Solve는 요청 x에 직접 도착하도록 적분합니다. TIME/PHASE TRACE 이동은
기존 258점 캐시의 화면용 선형 보간이며 tolerance 정확도를 보장하지 않습니다.
RK45는 explicit adaptive 방식이며 **stiff ODE 전용 solver가 아닙니다**.
강성·특이점·엄격한 tolerance에서는 Step underflow/Work limit에 도달할 수 있습니다.
허용오차는 전역 정확도 보증이 아닙니다. [계수·안전장치·정량 결과](RK45_NUMERICS.md)를 참고하세요.

## RK4 h·Step·Max Steps와 계산 시작 전 검사

h는 양수 유한값이어야 합니다. 0·음수·NaN·Inf는 거부합니다. 작은 h 자체를 일괄 금지하지 않으며,
각 IC에서 실제 적분할 방향 끝점까지 `ceil(abs(target-x0)/h)`를 계산합니다. 매우 작은 h는
정수 변환이나 적분 전에 차단합니다. `Max Steps`는 **각 IC의 각 방향** 상한이고 기본 20,000,
설정 가능 상한 100,000입니다. 전체 계산은 200,000 RK4 단계 및 600,000 `단계×차원` 이내여야 합니다.
OUTPUT OFF여도 모든 입력 IC와 모든 내부 state를 시작 전 작업량에 합산합니다.

- IC x0=0, 범위 -6..6, h=.1 또는 .001: 기본 Max Steps로 허용합니다.
- 같은 범위 h=.0001: 방향당 60,000단계여서 `Too many integration steps`와
  `Increase h or Max Steps`를 표시합니다. 설정을 보존하고 h 행으로 돌아갑니다.
- 범위 0...01, h=.0001: 약 100단계이므로 허용합니다.
- 전체 작업량 제한은 h를 늘리거나 범위·IC 수를 줄여 해결합니다. Max Steps만 늘려도 전체 제한은 유지됩니다.

`Step`은 출력 간격이며 RK4 h나 계산 횟수를 줄이지 않습니다. pan/zoom/ORIG와 preflight가 h를
자동으로 변경하지 않습니다. 긴 허용 계산도 EXIT로 취소할 수 있습니다. 이 제한은 소프트웨어
작업량 상한이며 fx-CG50의 최대 성능이나 특정 완료 시간을 보장하지 않습니다.

Separable `sinh(x)*(y^2-1)`, IC (0,0)의 해는 `-tanh(cosh(x)-1)`로 bounded입니다.
h=.1 RK4는 약 ±4.4에서 마지막 유효 수치점을 남기고 그 다음 단계에서 magnitude guard에 걸립니다.
이는 **고정 h의 수치 안정성 한계**이며 해의 수학적 발산으로 해석하면 안 됩니다.
RK4는 h를 자동 변경하지 않습니다. RK45 기본 tolerance에서는 x=7까지 완료했고 오차는 약1.78e-7입니다.
이는 관측한 구간의 검증이며 더 넓은 모든 구간에서 성공한다는 보장은 아닙니다.

## Table·STAT

Table은 항상 **작은 x → 큰 x 오름차순**이며 7행을 보여줍니다. 별도 DIR/IC+ mode는 없습니다.
**x는 맨 왼쪽에 고정**, ON 종속변수/해 열은 두 개씩 표시합니다. 세 열 이상일 때만
`Left/Right: columns` 안내를 표시합니다. LEFT/RIGHT는 열, UP/DOWN은 작은/큰 x 쪽 페이지입니다.

- **TOP**: 가장 작은 유효 x를 포함하는 page.
- **BTM**: 가장 큰 유효 x를 포함하는 page.
- **MID**: 처음 진입할 때 사용한 동일한 중앙 page. 반복 이동해도 같으며 수평 열 위치를 보존합니다.

유효 범위의 가운데 절반에 0이 있으면 x=0을 중앙 행에 놓습니다. 기존 출력 격자에 0이 없어도
RK4는 인접한 유효 점에서 보간해 0행을 추가하고, RK45는 x=0까지 직접 적분합니다. 그렇지 않으면 `(유효 min+max)/2`에 가장 가까운
sample을 중앙으로 사용합니다. h를 변경하지 않으며 범위 끝/7행 미만에서는 page를 안전하게 clamp합니다.
기본 ±6, h=.1의 첫 page는 -.3..+.3, 중앙이 0입니다. 비대칭 -2..8은 3 부근입니다.

1차 `y0={0,1}`은 **x | y1 | y2**, 세 값이면 y3까지 표시합니다. 여기의 y1/y2는 초기값 순서의
**해 번호**입니다. N-th의 y/y'/y(8), SYS의 y1/y2는 **state 변수**입니다. 이 두 의미를 혼용하지 않습니다.
각 해가 도달한 유효 범위가 다르면 도달하지 못한 셀은 `--`, CSV는 빈 셀입니다. 수락된 인접점 사이만
RK4에서 보간하며 실패한 점이나 빈 구간을 넘겨 값을 만들지 않습니다.
RK45 Table/STAT은 각 행 x까지 원래 IC에서 다시 적분하며 보간하지 않습니다. 재계산이 실패한 셀은 비웁니다. 수치 한계에서는 Step 격자 밖 마지막
수락점도 보존합니다. 긴 적분의 누적 x 반올림은 h의 1e-7 이하 차이 범위에서 수락점으로 맞춥니다.
부동소수점 표현 한계 때문에 출력 간격으로 서로 다른 x행을 만들 수 없으면 Table을 안전하게 거부합니다.
TOP/BTM의 정상 끝은 END, 해당 방향의 어느 해에 수치 한계가 있으면 `END: Numerical limit`을
그 page 안에 표시합니다. 별도 오류 modal이나 x0로의 점프는 없습니다.

**CSV/STAT은 항상 x가 첫 열**이며 전체 통합 범위와 ON 열을 내보냅니다. 현재 page나 수평 위치와
무관합니다. 모든 종속변수가 OFF이면 x만 있는 Table/CSV가 됩니다. STAT은 `DIFFSTAT00.csv`부터
첫 빈 이름에 저장합니다. 첫 행은 apostrophe로 시작하는 label입니다. 계산기 STAT List Editor에서
`F6 → F6 → CSV → LOAD → FILE`로 파일을 선택합니다.

```csv
'x,'y1,'y2
0,0,1
```

List Editor의 26열×999행 한도에 맞춰 label 1행 + **최대 998 data rows**입니다. 초과하면 파일을
생성하기 전에 거부합니다. RK4는 Step을 늘리고, RK45는 Solver Xrange를 줄이거나 TIME Xdot을 늘립니다. 전체 trajectory 대신 작은 범위 index와
7+1행 버퍼를 사용합니다. 각 파일 create/append/close는 `gint_world_switch` 안에서 완료하며,
취소/오류 파일은 handle이 정상적으로 닫혔을 때 제거합니다. **실제 STAT의 빈 셀 import/정렬 유지,
Fugue 저장/취소 반응은 HARDWARE RETEST REQUIRED**입니다. 빈값을 0으로 대체하지 않습니다.

## Recall과 저장

**[USER REQUESTED ADAPTATION]** Main **6 SAVE** 또는 SAVE 선택 후 EXE/F6 OPEN은 현재 식·IC·설정·Recall을 명시적으로
저장합니다. 아직 equation을 선택하지 않았다면 No session을 표시합니다. 일반 편집 확정,
NEXT/PREV/EXIT에서는 RAM만 유지하며 자동으로 session 파일을 쓰지 않습니다.

Main **5 RCL** 또는 RCL 선택 후 EXE/F6 OPEN에서 **Last calculation (RAM)** 또는 **Load saved session**을 선택합니다.
Main의 RECALL 텍스트 타일은 이 두 기능의 선택 화면을 엽니다.
Last calculation은 마지막으로 표시가 확정된 GRAPH 계산의 식·IC·Output을 복원하고
현재 app-wide Solver/V-Window와 Field Density/Style/Color는 유지합니다. Load saved session은 확인 후 저장된
current/recall과 설정을 모두 복원합니다. 취소된 Drawing은 Last calculation을 교체하지 않습니다. 진짜 새 실행은 defaults이며, 기존 파일은 삭제하지
않습니다. MENU 왕복으로 같은 실행이 재개될 때는 이 초기화를 다시 하지 않습니다.
Drawing 중 취소한 시도는 Last calculation을 바꾸지 않습니다. 처음 계산을 취소했다면
이전 계산 없음 상태를 유지하고, 이후 완성된 Graph나 유효한 부분 결과가 표시되면 Recall을 갱신합니다.

SAVE는 먼저 `Save current session?` 확인창을 엽니다. **F5 NO / EXIT**는 Main으로 돌아가며
파일 I/O가 없습니다. **F6 YES / EXE**만 기존 SAVE backend를 한 번 실행합니다. 확인창을 연 키의
hold/repeat는 승인으로 처리하지 않습니다. 성공/실패 안내 뒤에도 Main selector는 SAVE에 남습니다.
현재 세션이 없는 새 실행에서는 YES 이후 `No session.`을 표시하고 파일을 쓰지 않습니다.

계산기 root의 `DIFFEQ0.dat`와 `DIFFEQ1.dat`를 번갈아 씁니다. magic/version/size/checksum 및
값을 검사하고 최신 slot이 손상되면 다른 정상 slot을 사용합니다. 새 저장은 **v11**이며
Private Constants, 개별 G/L mask, x export flag는 없습니다. 메모리에 별도의 private constant 배열도 없습니다.

v11은 per-IC ON/OFF와 기존 색·Event·Method/RelTol/AbsTol을 current/recall에 저장합니다.
기존 v10 Event 설정과 v9 RK45 설정을 보존하며 v3~v10의 새 IC bits는 모두 ON으로 읽습니다.
v3~v9는 Event OFF로 읽으며 v9의 RK45 설정은 보존합니다. v3~v8은 RK4와 기본 tolerance로 읽습니다.
Diagnostics와 Event marker는 저장하지 않으며 load/recall 후 INFO는 No solver run yet입니다.
v8은 고정 layout reader로 독립 Phase 창, FIELD/NULL 표시 설정과 첫 창 준비 상태를 current/recall에서 보존합니다.
큰 trajectory나 analysis result는 저장하지 않습니다. v7의 10개 IC layout은 고정 reader로 읽고
Phase 설정을 초기화합니다. 옛 SYS 2D에서 phase가 켜져 있었다면 기존 shared 창을 Phase 창으로
옮기고 TIME 기본 창을 만듭니다. 자동/수동 여부와 무관하게 Solver 설정은 그대로 보존합니다.

v6의 9개 IC 저장 배열은 고정 layout으로 읽어 새 10개 배열에 복원합니다. current/recall과 설정·색을 보존하고 새 열은 초기화합니다.
같은 계산기 ABI의 v3/v4/v5도 각 버전의 layout으로 읽습니다. 이전 x OFF는 무시하고,
기존 G 또는 L에서 한 번이라도 ON인 dependent state는 모든 해에 ON으로 통합합니다.
v3/v4는 Arrow/Pale Blue, v5의 명시적 Segment/색은 유지합니다. v3 solution 색만 기본값을 부여합니다.
옛 상수 참조는 가능한 경우 괄호로 감싼 숫자식으로 치환합니다. 비유한 상수나 길이 초과면
원래 식을 유지하고 편집 필요를 알립니다. 새 parser는 이를 임의로 0으로 해석하지 않습니다.

옛 고차/SYS 여러 IC는 첫 완전 벡터를, 1차의 서로 다른 x0는 첫 x0와 같은 IC만 복원합니다.
값을 새 x0로 강제로 옮기지 않습니다. 이 적응이 필요하면 load 안내를 표시하며 **load는 원래 파일을
변경하지 않습니다**. 이후 명시적 SAVE는 새 v11으로 두 slot을 순환하므로, 여러 번 SAVE하면 옛
slot은 교체될 수 있습니다. 원본 보존이 필요하면 기존 파일을 따로 보관하십시오.
상세 mapping과 예외는 [OUTPUT/migration audit](OUTPUT_LIST_AUDIT.md)에 있습니다.

지원하지 않는 버전/손상 slot은 안전하게 건너뜁니다. raw ABI 형식이므로 host 테스트 `.dat`는
계산기로 이식할 수 없습니다. 실제 기기의 old-session 복구, 전원 차단 내구성과 MENU 복귀는
**HARDWARE RETEST REQUIRED**입니다.

## 바로 확인할 예제

1. **1st → Separable**의 기본값 f=1, g=y²−1, IC (0,0)은 감소하는 곡선을 그립니다. IC 화면에서 `x0=0, y0={0,1}`로 입력하면 수평 y=1도 함께 표시됩니다. 각 set은 독립된 초기값 문제입니다.
2. **2nd**의 기본값 f=0, g=1, h=0, 초기 y=1, y'=0은 cos(x)와 −sin(x)입니다.
3. **SYS → 2**의 기본값 y1'=y2, y2'=−y1, (1,0)에서 NEXT → NEXT → GRAPH → F4 VIEW → F2 PHASE → F5 ANLYS → F2 NULL → F3 EQPT로 oscillator phase를 봅니다.
4. **N-th → 3**에서 `sin(x)-y1-y2`를 입력하고 OPTN → Convert to system를 실행하면 `y1'=y2`, `y2'=y3`, `y3'=sin(x)-y2-y3`로 변환됩니다.

오차 확인 예제: 일반 1차 `y`, y(0)=1, x=1에서 h=.1의 RK4 값은 약 2.718279744입니다. h=.05, .025로 줄이면 host global error가 각각 약 1/15.35, 1/15.67로 줄었습니다. 목표 하드웨어 수학 라이브러리에서의 실제 값은 따로 검증해야 합니다.


## UI consistency (beta.3에서 유지한 규칙)

Graph 우측 위의 TIME/PHASE는 SYS2 VIEW가 있는 모드에서만 표시합니다. Event enabled이면 모든 모드에서 EVT를 표시합니다.
Field/nullcline/곡선 위에 Event square와 equilibrium diamond를 그리고, 작은 배경을 둔
legend/view/status 및 활성 커서·결과 footer를 조합합니다. END는 legend 아래 줄을 사용합니다.
TRACE/G-Solve 선택 곡선은 기존2px와 blink, Black↔Blue를 유지합니다. 결과 pointer가 footer
문자를 덮지 않습니다. Event marker의 화면 좌표 오프셋을 수정했으며 수치 root는 바뀌지 않았습니다.
입력 오류는 원래 title/field/draft를 유지하는 빨간 footer입니다. EXE/EXIT/F6 EDIT 후 수정합니다.
계산 종료는 유효한 곡선/Table과 함께 END/partial 이유를 표시하며, 긴 INFO·저장 결과는 기존
정보창을 사용합니다. SAVE/load/resize/convert 확인은 모두 **F5 NO/EXIT**, **F6 YES/EXE**이고
확인창을 연 키의 HOLD는 무시합니다. 현재 전체 F-key 표와 규칙은 [UI conventions](UI_CONVENTIONS.md),
[24개 renderer 화면](ui-review/consistency-overview.png)을 참고하십시오. 실기 화면은 아직 재검증 전입니다.


## Graph interaction / BOX (beta.4)

**Graph F6 INIT**는 이번 계산에서 해당 view가 처음 표시된 창의 범위·scale로 돌아갑니다.
TIME/PHASE 선택 자체는 유지합니다. V-WIN/Table/Settings를 다녀와도 원래 기준을 유지하며,
새 계산에서 기준을 다시 잡습니다. 식·IC·method/h/tolerance·Event·Output·manual range는
바꾸지 않고 AUTO 범위는 복구한 창을 따릅니다. 보관된 sample cache와 Event report가 재사용
가능하면 ODE 재적분 없이 그립니다. 캐시가 교체되었거나 범위가 부족하면 기존 안전한 redraw를
사용하므로 무조건 무계산이라고 보장하지 않습니다. **ZOOM ORIG**는 기존 factory 창입니다.

**F2 ZOOM → F5 BOX**는 가장 가까운 중앙 pixel(192,99)에서 시작합니다. 방향키/repeat로
4px씩 이동하고 EXE로 Point1을 고정합니다. Point2를 이동하면 pale-blue stipple과 파란 테두리가
보입니다. 두 번째 EXE가 최소 가로·세로6px인 사각형만 확정하고 Graph로 돌아갑니다. 모서리 순서는
무관합니다. 작은 영역은 BOX TOO SMALL로 유지하며 이동 후 다시 확정합니다. 어느 단계의 EXIT도
창을 바꾸지 않고 ZOOM으로 돌아갑니다. TIME/PHASE 창은 독립이며 manual Solver 범위를 보존합니다.

TRACE/G-Solve/BOX 위치는 동일한9px 검정 cross와 흰3px 중심으로 표시합니다. 긴 전 화면
crosshair는 제거했습니다. Event square·평형점 diamond와 선택 곡선2px/blink는 유지합니다.
G-Solve가 약156ms 이상 계속되면 파란 CALCULATING... spinner와 EXIT: cancel이 나타납니다.
실제 진행률을 표시하지 않으며 기존 EXIT/MENU 취소를 우선합니다. 결과·없음·오류·취소 모두에서
busy를 제거합니다. 빨간 END/Partial domain 경고는 전체 실패가 아니며 유효 구간의 TRACE와
G-Solve를 계속 사용할 수 있습니다. 기존 수치 종료 판정이나 wording은 바꾸지 않았습니다.

**Equation F1 INIT**는 현재 모드 식과 Bernoulli n만, **IC F1 INIT**는 현재 모드 초기조건만
새 문서의 기본값으로 복구하고 첫 행에 머뭅니다. FUNC/VAR token palette가 열려 있으면 palette
키가 우선하므로 EXIT로 닫은 뒤 INIT합니다. Equation은 F2 FUNC/F3 VAR가 EDIT에서만 보입니다.

Equation/IC의 빈칸·부분 식은 EXE/EXIT/행 이동으로 draft에 보관할 수 있습니다. 부모 EXIT는
검증하지 않습니다. **NEXT 또는 SELECT EXE**에서 모든 항목을 검사하고 첫 오류를 선택하여
빨간 inline 안내를 표시합니다. INIT와 입력 키는 계속 사용할 수 있습니다. IC는 전 항목이
유효할 때만 numeric IC에 반영하며, 뒤로 갔다 와도 draft를 유지합니다. 미확정 IC draft는
runtime UI 상태이고 SAVE v10에 추가하지 않았습니다. Equation의 raw text도 계산 전 검증하며
유효하지 않은 compiled 식을 solver에 넘기지 않습니다. Parameters→GRAPH의 기존 검증은 유지합니다.

[전체 감사·제약](INTERACTION_AUDIT.md), [28개 renderer 화면](ui-review/interaction-overview.png).
**HARDWARE RETEST REQUIRED:** LCD·키 반복·busy 응답·BOX·실기 저장 재검증 전입니다.
