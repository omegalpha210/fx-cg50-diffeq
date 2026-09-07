# DIFF EQ 사용 설명서 — v0.9.0-beta.3

안정 기준선 `0de88c7`을 보존하면서 TRACE X/Y 추적과 endpoint 이동, Graph Settings 조작, 1차 초기값 최대 10개를 추가했습니다. 이번 변경은 **HARDWARE TEST REQUIRED**이며 호스트 검증이 계산기 시험을 대신하지 않습니다. 실제 메뉴는 gint 기본 글꼴의 영어/ASCII를 사용합니다.

## 화면별 조작

| 화면 | F1 | F2 | F3 | F4 | F5 | F6 |
|---|---|---|---|---|---|---|
| 메인 | — | — | — | — | RCL | SAVE |
| Equation 기본 bar | VAR (필요한 모드만) | FUNC (EDIT만) | V-WIN | — | — | NEXT |
| Initial Conditions | PREV | — | V-WIN | — | — | NEXT |
| Solver Parameters | PREV | INIT | V-WIN | OUTPUT | SET | GRAPH (빨강) |
| Output 종속변수 | — | — | COLOR | INIT | — | DONE |
| Graph Settings: Grid/Label | — | — | — | INIT | — | DONE |
| Graph Settings: Style | SEG | ARROW | — | INIT | — | DONE |
| Graph Settings: Color | — | — | COLOR | INIT | — | DONE |
| FUNC 1쪽 | ABS | SINH | COSH | TANH | ASINH | 다음 |
| FUNC 2쪽 | ACOSH | ATANH | — | — | — | 이전 |
| 그래프 기본 화면 | TRACE | ZOOM | V-WIN | TABLE | G-SLV | PREV (magenta) |
| ZOOM 하위 메뉴 | IN | OUT | AUTO | ORIG | — | — |
| G-Solve 1쪽 | ROOT | MAX | MIN | Y-ICPT | ICPT | 다음 |
| G-Solve 2쪽 | Y-CAL | X-CAL | — | — | — | 이전 |
| Trace | x= | NORMAL | FAST | FASTER | LEFT | RIGHT |
| X-CAL / Y-CAL 숫자 입력 | — | — | — | — | — | RUN |
| Table | TOP | BTM | MID | — | STAT | GRAPH |

**[USER REQUESTED ADAPTATION]** 모든 모드는 Equation → F6 NEXT → IC → F6 NEXT →
Parameters → F6 GRAPH 순서입니다. 일반식은 Equation 상단, 실제 입력 행은 그 아래에 표시됩니다.
선택 상태에서 UP/DOWN은 이전/다음 입력 행을 선택합니다.
**편집 중이 아니면 EXE는 선택 행의 위치와 관계없이 F6 NEXT/GRAPH/DONE/OPEN을 실행합니다.**
따라서 기본 입력을 사용하면 Equation에서 EXE 세 번으로 Graph에 도달합니다.
LEFT는 편집 시작·커서 맨 앞, RIGHT는 편집 시작·커서 맨 뒤입니다. 직접 입력은 기존 값을 대체합니다.
편집 중 **EXE는 검증·확정 후 다음 행을 선택**하며, 다음 행의 편집까지 열지 않습니다.
마지막 행 편집에서는 첫 EXE가 확정 후 같은 행을 선택하고, 두 번째 EXE가 F6 동작을 실행합니다.
편집 중 **EXIT는 검증·확정 후 같은 행 선택**으로 돌아갑니다. 부모 화면에는 EXIT를 한 번 더 누릅니다.
잘못된 입력은 오류 안내 후 같은 draft·커서로 돌아옵니다. X/Y-CAL의 EXE는 아래의 RUN 예외를 따릅니다.

Equation·IC·Parameters·V-Window·차수/변수 개수의 일반 입력을 편집할 때 하단 한 줄은
`EXE: commit / next   EXIT: commit`입니다. 커서가 blink로 숨겨져도 EDIT 상태이면 유지합니다.
편집을 확정하면 해당 화면의 안내로 돌아갑니다. SELECT에는 EDIT 안내나 일반적인
`UP/DOWN: select`를 표시하지 않습니다. Table의 페이지 이동, TRACE의 곡선 전환,
G-Solve의 곡선 선택 등 특수 방향키 동작은 그대로입니다. 팝업·하위 메뉴의 전용 안내가
우선하고, 일반 폼에서는 EDIT 안내 → 화면별 필요한 안내 → 빈 줄 순서입니다.
Graph Settings의 ON/OFF·Style·Color 선택은 문자 입력 EDIT가 아니므로 전용 안내를 씁니다.
TRACE x=와 X/Y-CAL의 계산 입력은 각 화면에 표시된 기존 조작을 따릅니다.

NEXT/PREV와 사용 가능한 보조 softkey는 draft를 먼저 검증·확정한 뒤 이동합니다.
F3 V-WIN은 세 단계와 Graph에서 일관됩니다. OUTPUT과 SET은 Parameters에 있습니다.
보조 화면 EXIT는 진입 단계·선택 행으로 복귀합니다. SET은 중간 메뉴 없이 Graph settings를 바로 엽니다. Private Constants는 제거되었습니다. On/Off는 LEFT/RIGHT로 전환하고 EXE로 DONE을 실행합니다.
PREV는 magenta, NEXT는 cyan, V-WIN은 orange, SET은 bright green입니다.
실제 계산을 실행하는 F6 GRAPH는 red입니다.

Graph 기본 F6 PREV는 기존 magenta 스타일이며 Solver Parameters로 돌아갑니다.
TRACE는 별도 BACK 없이 EXIT로 닫습니다. ZOOM/G-Solve의 기존 bar를 유지하며, 하위 메뉴를 EXIT로 닫으면 PREV가 복원됩니다.
Main 하단은 `MENU: return to MAIN MENU`이며 MENU의 실제 계산기 Main Menu 복귀 동작은 동일합니다.
EXIT 순서는 Graph → Parameters → IC → Equation → subtype/order → Main입니다.
2nd 및 Main에서 RCL로 연 Equation은 Main으로 돌아갑니다. Main의 EXIT는 Main에 머뭅니다.
계산기 메뉴에는 **MENU**로 이동합니다. 같은 실행으로 복귀하면 현재 입력을 유지합니다.
진짜 새 실행은 defaults로 시작하며 저장 파일을 자동으로 읽지 않습니다.

Main의 숫자 **1/2/3/4**는 각각 1st/2nd/N-th/SYS입니다. Main의 F1~F4는 표시도 동작도 없습니다.
UP/DOWN·EXE 선택과 F5 RCL/F6 SAVE는 유지되며 숫자 5/6은 저장/Recall을 실행하지 않습니다.
일반 선택 메뉴는 UP/DOWN, EXE 또는 F6 OPEN, 숫자 shortcut을 지원합니다.
N-th/SYS 개수는 1~9 한 자리 정수를 입력한 뒤 첫 EXE로 확정하고 **두 번째 EXE 또는 F6 OPEN**으로 엽니다.
EXIT 편집 확정은 화면에 머뭅니다. 잘못된 값은 오류를 표시합니다.
새 기본 문서의 첫 크기 지정과 같은 크기 재진입은 확인창이 없습니다. 실제 식·IC 등의
사용자 데이터가 있는 문서의 크기 변경은 Change Equation Size를 표시합니다.
**F5 NO 또는 EXIT**는 기존 데이터를 유지하고, **F6 YES**만 기본 식·IC로 교체합니다.

계산 중 EXIT 또는 AC로 중단 요청을 보낼 수 있습니다. 마지막으로 수락된 점까지의 결과가 남으며 중단 상태가 표시됩니다.

## 식과 숫자 입력

세 단계 편집 중 좌/우는 커서를 이동하고 **DEL**은 왼쪽 문자를, **AC**는 전체를 지웁니다.
Equation의 F1 VAR/F2 FUNC는 현재 식 화면을 유지하고 **하단 bar만 교체**합니다.
메뉴 열기·페이지 전환은 draft·커서·선택을 보존합니다. F1~F5로 token을 현재 위치에
삽입하면 기본 bar로 돌아옵니다. **FUNC는 실제 EDIT 상태에서만 표시**하며 SELECT의 빈 F2는 무효입니다.
커서 blink의 ON/OFF와는 무관합니다. **FUNC/VAR에서 첫 EXIT는 submenu만 닫고** draft·커서·선택·EDIT를 유지합니다.
미완성 `sinh(`도 이때 검증하지 않습니다. 다음 EXIT가 평소의 검증·확정, 그 다음 EXIT가 부모 이동입니다.
길게 누른 EXIT 반복 이벤트는 이 계층을 연속으로 닫지 않습니다. EXE/방향키는 메뉴를 닫고 기존 편집 동작을 수행하며,
F6는 submenu 페이지 전환만 수행합니다. VAR는 N-th의 2~9차 도함수 별칭과 SYS의 y1~y9에만 표시합니다.
모든 1차 모드·선형 2차·N-th 1차에서는 F1이 비어 있으며 눌러도 팝업이 열리지 않습니다.
V-Window 편집의 CLEAR/DEL 등은 표시된 기능을 따릅니다. TRACE x=는 물리 숫자·함수·DEL/AC로 입력하고 EXE/EXIT로 확정합니다.

X-CAL/Y-CAL은 **매번 빈 입력·깜빡이는 커서**로 시작합니다. 숫자, 소수점, 음수와 EXP의 과학적 표기를 입력한 뒤
**EXE 또는 F6 RUN 한 번**으로 검증과 계산을 실행합니다. F1~F5는 비어 있습니다.
빈 값/잘못된 값은 `Invalid number`와 draft를 유지하며 이전 값을 재사용하지 않습니다.
유효한 편집 중 EXIT는 확정 후 머물고 다음 EXIT가 취소합니다. 오류 draft도 검증 규칙을 따릅니다.
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
모든 IC 화면의 F4/F5는 비어 있고 동작하지 않습니다. F3 V-WIN, F6 NEXT는 유지됩니다.

**Solver Parameters**의 x min/max는 적분 구간입니다. 처음에는 `ceil(V-Window Xmin)`과
`floor(V-Window Xmax)`이며 사용자가 V-Window/Graph pan/zoom을 조작하면 따라갑니다. TRACE runtime 추적은 이 자동 범위도 변경하지 않습니다. 직접 편집한 범위는 user override로
유지됩니다. **SF(0~100, 기본값 12)는 scalar 1차 네 모드에서만 Parameters에 표시합니다.**
Separable/Linear/Bernoulli/General은 Step → SF → Max steps 순서입니다.
2nd/N-th/SYS는 차수·변수 개수 1을 포함하여 SF 행이 없으며 Step → Max steps로 이동합니다.
UP/DOWN, 편집 EXE의 다음 행, 선택 행 도움말도 표시되는 행만 따릅니다.
Parameter F2 INIT는 자동 범위 추종, h=.1, Step=1, Max Steps=20000을 복구합니다.
1차에서는 SF도 12로 복구하며, 고차/SYS에서는 숨겨진 SF 값을 보존합니다.
예를 들어 SF=20 → 2nd → INIT → 1st에서도 20입니다. SAVE/RCL도 숨겨진 SF를 보존하며
새 저장은 v7이며 기존 v3~v6 세션의 읽기 호환성을 유지합니다. Field Style/Color는 모든 모드에서
보존합니다. Solver 수정은 V-Window를 역으로 변경하지 않습니다.
Xdot은 `(Xmax-Xmin)/378`이며 Xdot 편집은 Xmax를 변경합니다.

**OUTPUT에는 종속변수만 표시합니다.** 1차는 y, 2차는 y/y', N-th는 각 도함수, SYS는 y1~ym입니다.
x 행과 CSV X control은 없습니다. **LEFT/RIGHT는 선택 변수의 ON/OFF**, UP/DOWN은 행 이동입니다.
하단 안내는 `LEFT/RIGHT: ON/OFF toggle` 한 줄입니다. 7행 페이지에서는 위쪽의 부가 설명을
생략해 마지막 행과 겹치지 않으며, F3 COLOR·INIT·DONE과 기존 EXE 동작은 유지됩니다.
한 ON/OFF 값이 모든 IC에 공통으로 적용됩니다. ON은 Graph·TRACE·G-Solve·Table·CSV/STAT에 포함,
OFF는 이 사용자 출력들에서 숨김입니다. 숨겨진 y' 같은 내부 state도 RK4에서 계속 계산합니다.
Phase는 두 축의 state가 모두 ON일 때 표시합니다. 원래 독립 G/L 선택이 제공하던 차이와
이번 통합의 호환성 정책은 [OUTPUT audit](OUTPUT_LIST_AUDIT.md)에 기록했습니다.

색은 선택 행에서 **F3 COLOR**로만 엽니다. 방향키로 고르고 EXE 적용, EXIT 취소입니다.
지원 색상은 Blue, Red, Magenta, Black, Cyan, Bright Green(0x37e6)이며 기존 palette를 유지합니다.
기본 순서는 Magenta → Cyan → Bright Green → Red → Blue → Black이고 7번째부터 반복합니다.
1차의 여러 IC는 이 순서로 자동 순환합니다. OUTPUT의 y 색은 첫 해의 색과 순환 시작점을 정하므로
모든 해를 같은 색으로 만들지 않습니다. 고차/SYS는 각 state의 선택 색을 사용합니다.
OUTPUT EXE는 다음 행으로 이동하며 마지막 행은 수정했으면 한 번 확정/머묾, 다음 EXE가 DONE입니다.
수정하지 않은 마지막 행은 곧바로 DONE입니다. F4 INIT는 모든 종속변수 ON과 기본 solution 색,
첫 선택 행을 복구하며 SF/field 외형은 유지합니다.

V-WIN INIT, Parameter INIT, OUTPUT INIT, Graph Settings INIT는 값을 복구하고 화면에
머물며 첫 항목 선택·편집 종료 상태가 됩니다. Parameters F5 SET은 Graph Settings로 직접 들어가고
EXIT는 정확한 이전 Parameters 선택 행으로 돌아갑니다.

## 기울기장 — 밀도는 Parameters, 외형은 Graph Settings

**SF=0은 OFF, 1~100은 화면 열 수**입니다. 세로 행 수는 화면 비율에 맞추며 SF12는 12×7,
SF100은 100×52점입니다. SF는 h/Step/적분 구간/IC/TRACE 이동 간격을 바꾸지 않습니다.

Graph Settings는 Grid / Axis Label와 간격을 둔 Slope Field heading 아래 Style / Color만 둡니다.
Grid/Axis Label은 LEFT/RIGHT로만 토글하며 F1/F2는 비어 있고 동작하지 않습니다.
이 두 행에서만 `LEFT/RIGHT: ON/OFF toggle` 도움말을 표시합니다. EXE의 완료 동작은 동일합니다.
고정 `Density SF: Parameters` 안내는 없으며 Style/Color에서는 불필요한 하단 안내를 표시하지 않습니다.
Style은 F1 SEG/F2 ARROW 또는 좌우, Color는 좌우/F3로 선택합니다. 기본은 **Arrow / Pale Blue**입니다.
Color 행은 이름 옆에 실제 선택색의 직사각형 견본을 표시합니다. OUTPUT과 같은 테두리·크기를
사용하며 Pale Blue/Red/Cyan/Magenta/Gold/Gray의 기존 색상표와 선택·취소 동작을 유지합니다.
Graph Settings F4 INIT는 Grid/Axis Label ON, Arrow/Pale Blue를 복구하고 **SF는 바꾸지 않습니다**.
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
- `+`/`−` 또는 ZOOM: 중앙 기준 확대/축소. Solver range를 직접 override하지 않은 상태에서는 새 X window에 맞춰 적분 구간도 갱신됩니다.
- ZOOM은 현재 그래프 위에서 F1 IN / F2 OUT / F3 AUTO / **F4 ORIG**를 표시합니다. 각 동작 후에도 ZOOM bar를 유지하며 **EXIT로만 닫습니다**. ZOOM 안에서도 방향키로 pan하며 메뉴가 유지됩니다. 메뉴를 열고 닫기만 하면 재적분·파일 I/O가 없습니다.
- ORIG는 V-WIN INIT와 같은 기본 창으로 돌아갑니다. 식·IC·h는 유지하며 Solver AUTO만 새 창을 따릅니다. 수동 Solver 범위는 유지됩니다.
- AUTO는 현재 X 범위를 유지하고, Solver와 X 창의 교집합에 있는 ON 상태의 곡선의 유한한 값으로 Y 범위를 맞춥니다. 현재 Y 창 밖의 값도 사용하며 상수 곡선에도 여백을 줍니다. 유효한 점이 없거나 계산이 중단되면 기존 창을 보존하고 하단에 안내합니다. Phase에서는 가로 상태 창을 유지하고 그 안의 세로 상태 값을 사용합니다.
- OPTN 물리 키: phase 전환, 가로/세로 상태 선택, auto window, Grid/Axis Label 설정, 현재 범위/계산 상세.
- TRACE: **F2 NORMAL / F3 FAST / F4 FASTER**는 한 번의 유효 LEFT/RIGHT 이동을 각각
  **1× / 2× / 3× 실제 Xdot**으로 정합니다. `Xdot=(Xmax-Xmin)/378`이며 표시 문자열의 반올림값을 쓰지 않습니다.
  x=.6, Xdot=.025이면 다음 RIGHT는 .625/.65/.675입니다. NORMAL은 노랑, FAST는 Bright Green, FASTER는 Cyan 배경이며 모두 검정 글씨입니다. 선택 버튼의 검정 테두리로 현재 속도를 표시하고 배경색은 유지합니다.
  모드 버튼 자체는 현재 x·곡선·blink·창·h·Step을 바꾸거나 solver를 실행하지 않습니다.
  새 TRACE는 NORMAL이며 속도는 저장하지 않습니다. 숫자 x= 편집 중에는 속도 버튼이 숨겨집니다.
  저장된 유효
  RK4 점 사이를 선형 보간합니다. 캐시 내부의 x 이동과 blink는 적분하지 않습니다. **표시되는 y는 보간 근사값**이며
  정확한 지정 x의 RK4 결과가 필요하면 F1 x=를 사용합니다. 보간은 invalid gap을 건너지 않습니다.
  UP/DOWN은 ON 곡선을 바꾸고 가능한 경우 같은 x를 유지합니다. Phase도 같은 독립변수 x 간격을 사용하되 자동 pan/범위 확장은 하지 않습니다.
  **F5 LEFT / F6 RIGHT**는 현재 설정된 Solver Xrange min/max로 이동합니다. 캐시의 유효 endpoint/보간을
  우선하며 numerical-invalid endpoint에서는 가장 가까운 유효 점과 짧은 numerical-limit 안내를 사용합니다.
  현재 곡선·속도는 유지되고, runtime 계산 범위가 넓어져도 점프 기준은 설정 범위입니다.
  유효한 커서가 X 또는 Y 여백 10%를 넘으면 해당 축을 평행 이동하여 30% 내부에 둡니다.
  두 축이 동시에 필요하면 한 번에 갱신하고 한 번만 다시 그립니다. X/Y span·scale·h·Step은 유지합니다.
  NaN/Inf, 절댓값 1e100 초과, invalid gap의 가짜 값으로 창을 옮기지 않습니다.
  **설정 Solver Xrange / runtime 계산 범위 / 표시 V-Window는 별개**입니다.
  TRACE 이동은 자동/수동 Solver Xrange를 모두 보존합니다.
  runtime [-6,6]에서 x=5.9, x=6, F6 RIGHT는 추가 적분하지 않습니다. 실제 요청 target이 범위를
  벗어날 때만 그 target까지 기존 preflight와 제한 아래 캐시를 재계산합니다. 여분 범위를 미리 계산하지 않습니다.
  예를 들어 x=5.95, Xdot=.1, FASTER RIGHT는 같은 입력에서 6.25까지 확장·이동합니다.
  캐시 밖의 연속 이동은 매번 계산이 필요할 수 있으며 한 작업만 진행하고 repeat를 병합합니다.
  취소·work-limit 실패 시 임시 결과를 버려 이전 캐시·커서·창·완성 그래프를 보존합니다.
  캐시는 표시 중인 모든 IC에 **총 258점**을 나누어 사용하므로, 긴 구간이나 많은 IC에서는
  TRACE 보간·pan 후 곡선의 해상도가 낮아질 수 있습니다. 보관량과 RK4 단계 수는 별개입니다.
  선택 곡선은 250 ms timer로 강조/해제됩니다. Black은 **Black↔Blue**이며 G-Solve도 같은 정책입니다.
  LEFT/RIGHT hold는 세 모드 모두 400 ms 후 일정한 125 ms(8회/초) 간격입니다.
  FAST/FASTER는 이동 간격만 바꾸며 보간 정확도나 CPU/키 반복 속도를 높이지 않습니다.
  target x가 현재 x와 같아지는 극단적 범위나 non-finite target은 안전하게 거부합니다.
  중복 반복을 합치고 EXIT/MENU를 우선합니다. 확장 중 취소하면 이전 캐시·그래프·창을 유지합니다.
  정상 pan 후 EXIT는 현재 이동한 창을 유지하며 Graph bar로 돌아갑니다. 실제 LCD 해상도·반복키·취소 반응은
  **HARDWARE TEST REQUIRED**입니다.


G-SLV 메뉴를 열거나 페이지를 바꿨다가 EXIT만 하면 그래프와 계산 횟수가 유지됩니다. G-SLV의 수치 범위는 현재 V-Window X 범위와 Solver 범위의 교집합입니다. Ymin/Ymax는 그리기와 pointer clipping에만 사용됩니다. ROOT, MAX/MIN, Y-ICPT(x=0), Y-CAL, X-CAL과 두 보이는 곡선의 교점 ICPT는 화면 밖 y도 계산하고 좌표를 하단에 표시합니다. G-Solve menu에서 방향키는 pan이고, operation을 고른 뒤 현재 candidate가 깜빡이며 UP/DOWN은 blink 대상을 옮깁니다. EXE는 곡선을 확정하고 LEFT/RIGHT는 여러 결과를 이동합니다. 보이는 곡선이 정확히 둘이면 ICPT가 자동 선택하며 더 많으면 Curve A와 B를 각각 고릅니다. 결과 없음과 오류는 dialog 없이 같은 graph 하단에 표시됩니다. 접하는 근, h 사이의 매우 빠른 진동, 특이점과 불완전 trajectory는 놓칠 수 있습니다.

Phase portrait는 두 상태 이상일 때 사용합니다. 가로·세로로 서로 다른 상태를 선택합니다. 원본은 STAT의 List plot으로 phase를 얻었으며 이 직접 화면은 **[EXTENSION]**입니다. 독립변수 x는 계속 적분 변수이고 Trace의 UP/DOWN은 표시 중인 trajectory family를 바꿉니다. 가로/세로 pixel scale이 다르면 oscillator의 원 궤적이 화면에서 타원처럼 보일 수 있습니다.

### 수치 guard와 유효한 구간

`|value| > 1e100`은 application guard, NaN/Inf와 수학 domain 오류는 별도 원인입니다.
guard는 유지하며 실패한 점을 그리거나 빈 구간 양쪽을 선으로 연결하지 않습니다.
Graph의 `ERROR: Magnitude > 1e100` 등은 계산된 유효한 구간을 폐기하지 않습니다.
명시적 사용자 중단은 별도의 `Partial: Cancelled`입니다.

TRACE는 남아 있는 유효 점에서 계속 이동하며 경계에서는 `TRACE: Numerical limit`을 표시합니다.
신뢰할 수 있는 다음 구간이 제공되면 가장 가까운 다음 유효 점으로 이동합니다. X-CAL/ROOT/MAX/MIN/ICPT는
계산된 유효 구간에서만 찾고 x순으로 결과를 표시하며, 빈 구간을 가로질러 근·극값을 만들지 않습니다.
Y-CAL과 TRACE x=는 요청한 x까지 해당 IC에서 적분할 수 있을 때만 값을 반환합니다.

**현재 RK4는 각 IC에서 양방향으로 성공한 prefix를 보존합니다.** 실패 지점 너머의 같은 해를
새 초기조건 없이 재시작하지 않습니다. 반대 방향과 별도 IC는 계속 사용할 수 있지만 별도 IC는 별도 곡선입니다.
합성 valid/gap/valid 테스트는 신뢰 가능한 양쪽 값이 주어졌을 때의 소비 경로를 검증하며,
일반 ODE가 특이점을 통과했다는 뜻이 아닙니다. 결과 없음은 계산되지 않은 영역의 해 부재를 보장하지 않습니다.
자세한 경계는 [Numerical validity audit](NUMERICAL_VALIDITY_AUDIT.md)에 있습니다.

## h·Step·Max Steps와 계산 시작 전 검사

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
작은 h를 사용자가 선택할 수 있지만 이번 버전은 adaptive solver나 자동 재시도를 하지 않습니다.

## Table·STAT

Table은 항상 **작은 x → 큰 x 오름차순**이며 7행을 보여줍니다. 별도 DIR/IC+ mode는 없습니다.
**x는 맨 왼쪽에 고정**, ON 종속변수/해 열은 두 개씩 표시합니다. 세 열 이상일 때만
`Left/Right: columns` 안내를 표시합니다. LEFT/RIGHT는 열, UP/DOWN은 작은/큰 x 쪽 페이지입니다.

- **TOP**: 가장 작은 유효 x를 포함하는 page.
- **BTM**: 가장 큰 유효 x를 포함하는 page.
- **MID**: 처음 진입할 때 사용한 동일한 중앙 page. 반복 이동해도 같으며 수평 열 위치를 보존합니다.

유효 범위의 가운데 절반에 0이 있으면 x=0을 중앙 행에 놓습니다. 기존 출력 격자에 0이 없어도
인접한 유효 RK4 점에서 보간해 0행을 추가합니다. 그렇지 않으면 `(유효 min+max)/2`에 가장 가까운
sample을 중앙으로 사용합니다. h를 변경하지 않으며 범위 끝/7행 미만에서는 page를 안전하게 clamp합니다.
기본 ±6, h=.1의 첫 page는 -.3..+.3, 중앙이 0입니다. 비대칭 -2..8은 3 부근입니다.

1차 `y0={0,1}`은 **x | y1 | y2**, 세 값이면 y3까지 표시합니다. 여기의 y1/y2는 초기값 순서의
**해 번호**입니다. N-th의 y/y'/y(8), SYS의 y1/y2는 **state 변수**입니다. 이 두 의미를 혼용하지 않습니다.
각 해가 도달한 유효 범위가 다르면 도달하지 못한 셀은 `--`, CSV는 빈 셀입니다. 수락된 인접점 사이만
보간하며 실패한 점이나 빈 구간을 넘겨 값을 만들지 않습니다. 수치 한계에서는 Step 격자 밖 마지막
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
생성하기 전에 거부하고 Step을 늘리도록 안내합니다. 전체 trajectory 대신 작은 범위 index와
7+1행 버퍼를 사용합니다. 각 파일 create/append/close는 `gint_world_switch` 안에서 완료하며,
취소/오류 파일은 handle이 정상적으로 닫혔을 때 제거합니다. **실제 STAT의 빈 셀 import/정렬 유지,
Fugue 저장/취소 반응은 HARDWARE RETEST REQUIRED**입니다. 빈값을 0으로 대체하지 않습니다.

## Recall과 저장

**[USER REQUESTED ADAPTATION]** Main F6 SAVE는 현재 식·IC·설정·Recall을 명시적으로
저장합니다. 아직 equation을 선택하지 않았다면 No session을 표시합니다. 일반 편집 확정,
NEXT/PREV/EXIT에서는 RAM만 유지하며 자동으로 session 파일을 쓰지 않습니다.

Main F5 RCL에서 **Last calculation (RAM)** 또는 **Load saved session**을 선택합니다.
Main의 짧은 설명 `Recall / load session`은 이 두 기능을 함께 나타냅니다.
Last calculation은 기존 의미를 유지해 마지막 GRAPH 계산의 식·IC·Output을 복원하고
현재 app-wide Solver/V-Window와 Field Density/Style/Color는 유지합니다. Load saved session은 확인 후 저장된
current/recall과 설정을 모두 복원합니다. 진짜 새 실행은 defaults이며, 기존 파일은 삭제하지
않습니다. MENU 왕복으로 같은 실행이 재개될 때는 이 초기화를 다시 하지 않습니다.

계산기 root의 `DIFFEQ0.dat`와 `DIFFEQ1.dat`를 번갈아 씁니다. magic/version/size/checksum 및
값을 검사하고 최신 slot이 손상되면 다른 정상 slot을 사용합니다. 새 저장은 **v7**이며
Private Constants, 개별 G/L mask, x export flag는 없습니다. 메모리에 별도의 private constant 배열도 없습니다.

v6의 9개 IC 저장 배열은 고정 layout으로 읽어 새 10개 배열에 복원합니다. current/recall과 설정·색을 보존하고 새 열은 초기화합니다.
같은 계산기 ABI의 v3/v4/v5도 각 버전의 layout으로 읽습니다. 이전 x OFF는 무시하고,
기존 G 또는 L에서 한 번이라도 ON인 dependent state는 모든 해에 ON으로 통합합니다.
v3/v4는 Arrow/Pale Blue, v5의 명시적 Segment/색은 유지합니다. v3 solution 색만 기본값을 부여합니다.
옛 상수 참조는 가능한 경우 괄호로 감싼 숫자식으로 치환합니다. 비유한 상수나 길이 초과면
원래 식을 유지하고 편집 필요를 알립니다. 새 parser는 이를 임의로 0으로 해석하지 않습니다.

옛 고차/SYS 여러 IC는 첫 완전 벡터를, 1차의 서로 다른 x0는 첫 x0와 같은 IC만 복원합니다.
값을 새 x0로 강제로 옮기지 않습니다. 이 적응이 필요하면 load 안내를 표시하며 **load는 원래 파일을
변경하지 않습니다**. 이후 명시적 SAVE는 새 v7로 두 slot을 순환하므로, 여러 번 SAVE하면 옛
slot은 교체될 수 있습니다. 원본 보존이 필요하면 기존 파일을 따로 보관하십시오.
상세 mapping과 예외는 [OUTPUT/migration audit](OUTPUT_LIST_AUDIT.md)에 있습니다.

지원하지 않는 버전/손상 slot은 안전하게 건너뜁니다. raw ABI 형식이므로 host 테스트 `.dat`는
계산기로 이식할 수 없습니다. 실제 기기의 old-session 복구, 전원 차단 내구성과 MENU 복귀는
**HARDWARE RETEST REQUIRED**입니다.

## 바로 확인할 예제

1. **1st → Separable**의 기본값 f=1, g=y²−1, IC (0,0)은 감소하는 곡선을 그립니다. IC 화면에서 `x0=0, y0={0,1}`로 입력하면 수평 y=1도 함께 표시됩니다. 각 set은 독립된 초기값 문제입니다.
2. **2nd**의 기본값 f=0, g=1, h=0, 초기 y=1, y'=0은 cos(x)와 −sin(x)입니다.
3. **SYS → 2**의 기본값 y1'=y2, y2'=−y1, (1,0)에서 NEXT → NEXT → GRAPH → OPTN → phase 전환 → ZOOM → AUTO로 oscillator phase를 봅니다.
4. **N-th → 3**에서 `sin(x)-y1-y2`를 입력하고 OPTN → Convert to system를 실행하면 `y1'=y2`, `y2'=y3`, `y3'=sin(x)-y2-y3`로 변환됩니다.

오차 확인 예제: 일반 1차 `y`, y(0)=1, x=1에서 h=.1의 RK4 값은 약 2.718279744입니다. h=.05, .025로 줄이면 host global error가 각각 약 1/15.35, 1/15.67로 줄었습니다. 목표 하드웨어 수학 라이브러리에서의 실제 값은 따로 검증해야 합니다.
