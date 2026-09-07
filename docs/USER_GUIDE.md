# DIFF EQ 사용 설명서 — v0.9.0-beta.1

안정 기준선 `6addf4d`의 기능을 보존하고 실제 하드웨어 피드백을 반영했습니다. 이번 EXE·메뉴·유효 구간 변경은 **HARDWARE TEST REQUIRED**이며, 호스트 검증이 계산기 시험을 대신하지 않습니다. 앱의 실제 메뉴는 gint 기본 글꼴로 표시할 수 있는 영어/ASCII를 사용합니다.

## 화면별 조작

| 화면 | F1 | F2 | F3 | F4 | F5 | F6 |
|---|---|---|---|---|---|---|
| 메인 | 1st | 2nd | N-th | SYS | RCL | SAVE |
| Equation 기본 bar | VAR | FUNC | V-WIN | — | — | NEXT |
| Initial Conditions | PREV | — | V-WIN | ADD | DROP | NEXT |
| Solver Parameters | PREV | INIT | V-WIN | OUTPUT | SET | GRAPH (빨강) |
| Output matrix | GRAPH (파랑) | LIST | COLOR | INIT | — | DONE |
| Graph Settings | ON | OFF | — | INIT | — | DONE |
| FUNC 1쪽 | ABS | SINH | COSH | TANH | ASINH | 다음 |
| FUNC 2쪽 | ACOSH | ATANH | — | — | — | 이전 |
| 그래프 | TRACE | ZOOM | V-WIN | TABLE | G-SLV | BACK |
| ZOOM 하위 메뉴 | IN | OUT | AUTO | ORIG | — | — |
| G-Solve 1쪽 | ROOT | MAX | MIN | Y-ICPT | ICPT | 다음 |
| G-Solve 2쪽 | Y-CAL | X-CAL | — | — | — | 이전 |
| Trace | x= | — | — | — | — | BACK |
| X-CAL / Y-CAL 숫자 입력 | — | — | — | — | — | RUN |
| Table | TOP | BTM | DIR | IC+ | STAT | GRAPH |

**[USER REQUESTED ADAPTATION]** 모든 모드는 Equation → F6 NEXT → IC → F6 NEXT →
Parameters → F6 GRAPH 순서입니다. 일반식은 Equation 상단, 실제 입력 행은 그 아래에 표시됩니다.
선택 상태에서 UP/DOWN은 이전/다음 입력 행, **EXE는 다음 입력 행**을 선택합니다.
**마지막 행이 이미 선택 상태이면 EXE는 화면의 F6 NEXT/GRAPH/DONE을 실행합니다.**
LEFT는 편집 시작·커서 맨 앞, RIGHT는 편집 시작·커서 맨 뒤입니다. 직접 입력은 기존 값을 대체합니다.
편집 중 **EXE는 검증·확정 후 다음 행을 선택**하며, 다음 행의 편집까지 열지 않습니다.
마지막 행 편집에서는 첫 EXE가 확정 후 같은 행을 선택하고, 두 번째 EXE가 F6 동작을 실행합니다.
편집 중 **EXIT는 검증·확정 후 같은 행 선택**으로 돌아갑니다. 부모 화면에는 EXIT를 한 번 더 누릅니다.
잘못된 입력은 오류 안내 후 같은 draft·커서로 돌아옵니다. X/Y-CAL의 EXE는 아래의 RUN 예외를 따릅니다.

NEXT/PREV와 사용 가능한 보조 softkey는 draft를 먼저 검증·확정한 뒤 이동합니다.
F3 V-WIN은 세 단계와 Graph에서 일관됩니다. OUTPUT과 SET은 Parameters에 있습니다.
보조 화면 EXIT는 진입 단계·선택 행으로 복귀합니다. SET은 Graph settings와 Private
constants입니다. On/Off는 LEFT/RIGHT로 전환하고 EXE로 다음 행/마지막 DONE을 실행합니다.
PREV는 magenta, NEXT는 cyan, V-WIN은 orange, SET은 bright green입니다.
실제 계산을 실행하는 F6 GRAPH는 red이며 OUTPUT F1 GRAPH는 일반 blue입니다.

EXIT 순서는 Graph → Parameters → IC → Equation → subtype/order → Main입니다.
2nd 및 Main에서 RCL로 연 Equation은 Main으로 돌아갑니다. Main의 EXIT는 Main에 머뭅니다.
계산기 메뉴에는 **MENU**로 이동합니다. 같은 실행으로 복귀하면 현재 입력을 유지합니다.
진짜 새 실행은 defaults로 시작하며 저장 파일을 자동으로 읽지 않습니다.

Main의 숫자 **1/2/3/4**는 각각 1st/2nd/N-th/SYS이며 기존 F1~F4와 같습니다.
UP/DOWN·EXE 선택과 F5 RCL/F6 SAVE는 유지되며 숫자 5/6은 저장/Recall을 실행하지 않습니다.
일반 선택 메뉴는 UP/DOWN, EXE 또는 F6 OPEN, 숫자 shortcut을 지원합니다. 긴 상수 목록은 PG-/PG+로 넘깁니다.
N-th/SYS 개수는 1~9 한 자리 정수를 입력한 뒤 첫 EXE로 확정하고 **두 번째 EXE 또는 F6 OPEN**으로 엽니다.
EXIT 편집 확정은 화면에 머뭅니다. 잘못된 값은 오류를 표시합니다.
새 기본 문서의 첫 크기 지정과 같은 크기 재진입은 확인창이 없습니다. 실제 식·IC 등의
사용자 데이터가 있는 문서의 크기 변경은 Change Equation Size를 표시합니다.
**F5 NO 또는 EXIT**는 기존 데이터를 유지하고, **F6 YES**만 기본 식·IC로 교체합니다.

계산 중 EXIT 또는 AC로 중단 요청을 보낼 수 있습니다. 마지막으로 수락된 점까지의 결과가 남으며 중단 상태가 표시됩니다.

## 식과 숫자 입력

세 단계 편집 중 좌/우는 커서를 이동하고 **DEL**은 왼쪽 문자를, **AC**는 전체를 지웁니다.
Equation의 F1 VAR/F2 FUNC는 현재 식 화면을 유지하고 **하단 bar만 교체**합니다.
메뉴 열기·페이지 전환·EXIT 닫기는 draft·커서·선택을 바꾸지 않습니다. F1~F5로 token을 현재
위치에 삽입하면 기본 bar로 돌아오며, 선택 커서가 없는 하위 메뉴의 EXE는 아무것도 삽입하지 않습니다.
V-Window/상수 편집의 CLEAR/DEL 등은 표시된 기능을 따릅니다. TRACE x=는 물리 숫자·함수·DEL/AC로 입력하고 EXE/EXIT로 확정합니다.

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
| 상수 | Equation OPTN → Insert private constant (편집 중 OPTN); SET → Private constants에서 값 지정 |

FUNC는 위 표의 두 페이지에 abs·쌍곡선·역쌍곡선 7개만 제공합니다. VAR의 N-th는
y1~y(N−1), SYS는 y1~yN이며, 일반 1차와 Separable의 g 행은 기존 y1 alias를 제공합니다.
x 전용 계수 행은 상태 변수가 없습니다. 물리 키의 x/y는 중복 추가하지 않습니다.

곱셈은 `2*x`, `A*y`, `sin(x)*y`처럼 명시합니다. `2x`, `xy`, `sin x`는 허용하지 않습니다. `^`는 오른쪽 결합이며 `-2^2=-4`, `2^3^2=512`입니다. 음수는 `-`로 표현하며 나눗셈은 `/`입니다. 각도는 OS 설정과 무관하게 radian입니다. 숫자 폼도 상수식 `pi`, `1/4`, `sqrt(2)`를 허용하지만 x/y 상태 변수는 허용하지 않습니다.

식의 변수 범위가 맞지 않거나 괄호가 빠지면 NEXT에서 식 이름과 문자 위치를 보여줍니다. 오류 draft를 수정하고 다시 NEXT 하세요. Parameters GRAPH에서 전체 문서를 다시 검증합니다. 숫자 단계의 OPTN은 함수/상수 삽입 메뉴입니다.

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

같은 종류·차수를 다시 선택하면 현재 작업을 유지합니다. 다른 종류/차수는 해당 모드의 예제 식과 IC로 바뀌지만 V-Window, Solver, Grid와 Axis Label은 app 전체 설정으로 유지됩니다. 1st editor의 EXIT는 subtype 목록, subtype EXIT는 Main으로 돌아갑니다. N-th/SYS editor의 EXIT는 차수 숫자 입력 화면으로 돌아갑니다.

## 초기조건·설정·출력

방정식 화면은 식과 계수만 표시하며 Solver step `h`는 표시하지 않습니다. 모든 모드의 일반식을 제목 바로 아래에 먼저 보여줍니다. F6 NEXT가 독립 Initial Conditions 화면을 열며, `IC 1`, `IC 2` block이 세로로 이어집니다. UP/DOWN으로 모든 block의 field를 순서대로 이동하고 LEFT/RIGHT 또는 숫자 키로 편집합니다. EXE는 다음 field를 선택합니다. N-th는 y와 N−1개 도함수 초기값을, SYS는 각 상태의 초기값을 한 block에 입력합니다.

**F4 ADD**는 현재 set의 초기값 벡터를 복사해 목록 끝에 추가하며 새 곡선의 기본 색상을 부여합니다(최대 9 set). **F5 DROP**은 선택 field의 block을 제거하고 색상·G/L 속성을 함께 이동합니다. 1차 모드에서 모두 제거하고 SF>0이면 slope field만 계산합니다. 고차/SYS 또는 SF=0에서는 마지막 IC 삭제를 막습니다. mode별 근거는 [IC behavior audit](IC_BEHAVIOR_AUDIT.md)에 있습니다.

**Solver parameters**의 x min/max는 적분 구간입니다. 처음에는 `ceil(V-Window Xmin)`과 `floor(V-Window Xmax)`이며 V-Window, pan, zoom을 따라갑니다. Solver X min/max를 직접 편집하면 user override가 되어 이후 window 변경에서 유지됩니다. Parameter F2 INIT는 자동 추종과 기본 h/Step/SF/Max Steps를 복구합니다. 정수 구간이 없는 좁은 창은 원래 창 범위로 안전하게 fallback합니다. Solver 수동 변경, h/Step/SF 수정과 NEXT/PREV는 V-Window를 역으로 바꾸지 않습니다. 선택 행의 하단에는 integration start/end, RK4 interval, output interval, slope-field density 또는 iteration limit 설명이 표시됩니다. Xdot은 `(Xmax-Xmin)/378`이며 Xdot 편집은 Xmax를 변경합니다.

Output은 변수 행과 IC set 열의 matrix입니다. 첫 위치 G는 그래프, 둘째 L은 앱 Table/STAT
export 선택입니다. `_`는 OFF입니다. 네 조합은 `G L`, `G _`, `_ L`, `_ _`이며 x는 그래프
선택 대상이 아니므로 `n L`/`n _`로 표시합니다(n: N/A). GRAPH와 LIST로 각각 전환하고,
**F4 INIT는 모든 IC의 G/L과 색상을 공통 기본값으로 복구하고 첫 변수·첫 IC를 선택**합니다. G OFF는 그리기·TRACE·G-Solve 후보에서 제외하고,
L OFF는 Table/STAT 열에서 제외합니다. 숨긴 y' 등도 ODE 적분에 필요한 state로 계속 계산합니다.
Phase 곡선은 가로·세로 두 상태의 G가 모두 ON일 때 표시됩니다.

각 상태의 G/L 오른쪽 작은 swatch가 COLOR입니다. 현재 변수에서 **RIGHT 또는 F3 COLOR**로
2행×3열 palette를 엽니다. 방향키로 고른 뒤 EXE로 적용, EXIT로 취소합니다.
F3는 흰 배경에 C 빨강/O 주황/L 밝은 초록/O cyan/R magenta로 표시됩니다.
OUTPUT 자체의 EXE는 palette나 G 토글을 열지 않고 다음 변수로 이동합니다.
마지막 변수가 수정되지 않았으면 즉시 DONE, G/L/색상을 수정했으면 첫 EXE가 확정 후 머물고
다음 EXE가 DONE입니다. 행/IC를 방향키로 바꾸면 적용된 설정은 유지됩니다.
지원 색상은 Blue, Red, Magenta, Black, Cyan, Bright Green입니다. Bright Green은
#33FF33에 가장 가까운 RGB565 `0x37e6`입니다. 기본 순서는 Magenta → Cyan → Bright
Green → Red → Blue → Black이며 7번째부터 반복합니다. 색상은 G/L 전환, 화면 왕복,
V-WIN, Graph, 명시적 SAVE/RCL에서 유지됩니다. Phase는 세로 상태의 색을 사용합니다.

V-WIN INIT, Parameter INIT, OUTPUT INIT, Graph Settings INIT는 값을 복구하고 화면에
머물며 **첫 항목 선택·편집 종료 상태**가 됩니다. Graph Settings INIT는 Grid/Axis Label만 기본 ON으로 돌립니다.

## 그래프·Trace·phase

컬러 곡선은 양방향으로 적분한 IC family를 약 2 px 두께로 표시합니다. 색은 OUTPUT의 IC·상태별 설정을 따릅니다. factory Grid와 Axis Label은 ON이며 저장된 OFF 값은 유지됩니다. 축·grid·slope field의 두께는 그대로입니다. 화면 밖 선분은 clipping하지만 적분은 계속합니다.

- 방향키: 표시 창을 폭/높이의 20%만큼 pan.
- `+`/`−` 또는 ZOOM: 중앙 기준 확대/축소. Solver range를 직접 override하지 않은 상태에서는 새 X window에 맞춰 적분 구간도 갱신됩니다.
- ZOOM은 현재 그래프 위에서 F1 IN / F2 OUT / F3 AUTO / **F4 ORIG**를 표시합니다. 각 동작 후에도 ZOOM bar를 유지하며 **EXIT로만 닫습니다**. 메뉴를 열고 닫기만 하면 재적분·파일 I/O가 없습니다.
- ORIG는 V-WIN INIT와 같은 기본 창으로 돌아갑니다. 식·IC·h는 유지하며 Solver AUTO만 새 창을 따릅니다. 수동 Solver 범위는 유지됩니다.
- AUTO는 현재 X 범위를 유지하고, Solver와 X 창의 교집합에 있는 G-selected 곡선의 유한한 값으로 Y 범위를 맞춥니다. 현재 Y 창 밖의 값도 사용하며 상수 곡선에도 여백을 줍니다. 유효한 점이 없거나 계산이 중단되면 기존 창을 보존하고 하단에 안내합니다. Phase에서는 가로 상태 창을 유지하고 그 안의 세로 상태 값을 사용합니다.
- OPTN 물리 키: phase 전환, 가로/세로 상태 선택, auto window, Grid/Axis Label 설정, 현재 범위/계산 상세.
- TRACE: LEFT/RIGHT는 선택 곡선의 보관된 RK4 점 사이를 이동합니다. 진입·곡선 전환 때
  한 IVP의 양방향 데이터를 준비하고 이후 이동/blink는 재적분하지 않습니다. 최대 129점씩
  보관하며 긴 구간은 h×Step 격자를 성기게 보관할 수 있습니다. 좌표는 실제 보관된 x입니다.
  x=는 지정 x를 한 번 정확히 적분하는 빠른 이동입니다. UP/DOWN은 G ON 곡선만 전환하며
  가까운 계산된 x를 선택합니다. 선택 곡선은 250 ms timer로 강조/해제됩니다.
  Black은 **Black↔Blue**로 깜빡이며 G-Solve 선택도 같은 정책입니다. 다른 TRACE 색의 기존 반전색은 유지됩니다.
  LEFT/RIGHT hold는 400 ms 이후 일정한 125 ms(8회/초) 간격이고 오래 눌러도 가속하지 않습니다.
  중복 반복은 합치며 EXIT/MENU를 우선합니다. TRACE EXIT는 계산 중이던 임시 데이터를 버리고
  진입 전 완성 그래프와 기본 softkey를 복원합니다. TRACE는 현재 계산 범위 안에서 이동하고,
  범위 확장은 기존 Graph 방향키 pan을 사용합니다. 실제 키 반응·timer 주기는 하드웨어 재시험 대상입니다.


G-SLV 메뉴를 열거나 페이지를 바꿨다가 EXIT만 하면 그래프와 계산 횟수가 유지됩니다. G-SLV의 수치 범위는 현재 V-Window X 범위와 Solver 범위의 교집합입니다. Ymin/Ymax는 그리기와 pointer clipping에만 사용됩니다. ROOT, MAX/MIN, Y-ICPT(x=0), Y-CAL, X-CAL과 두 보이는 곡선의 교점 ICPT는 화면 밖 y도 계산하고 좌표를 하단에 표시합니다. G-Solve menu에서 방향키는 pan이고, operation을 고른 뒤 현재 candidate가 깜빡이며 UP/DOWN은 blink 대상을 옮깁니다. EXE는 곡선을 확정하고 LEFT/RIGHT는 여러 결과를 이동합니다. 보이는 곡선이 정확히 둘이면 ICPT가 자동 선택하며 더 많으면 Curve A와 B를 각각 고릅니다. 결과 없음과 오류는 dialog 없이 같은 graph 하단에 표시됩니다. 접하는 근, h 사이의 매우 빠른 진동, 특이점과 불완전 trajectory는 놓칠 수 있습니다.

Phase portrait는 두 상태 이상일 때 사용합니다. 가로·세로로 서로 다른 상태를 선택합니다. 원본은 STAT의 List plot으로 phase를 얻었으며 이 직접 화면은 **[EXTENSION]**입니다. 독립변수 x는 계속 적분 변수이고 Trace의 UP/DOWN은 표시 중인 trajectory family를 바꿉니다. 가로/세로 pixel scale이 다르면 oscillator의 원 궤적이 화면에서 타원처럼 보일 수 있습니다.

### 수치 guard와 유효한 구간

`|value| > 1e100`은 application guard, NaN/Inf와 수학 domain 오류는 별도 원인입니다.
guard는 유지하며 실패한 점을 그리거나 빈 구간 양쪽을 선으로 연결하지 않습니다.
Graph의 `ERROR: Magnitude > 1e100` 등은 계산된 유효한 구간을 폐기하지 않습니다.
명시적 사용자 중단은 별도의 `Partial: Cancelled`입니다.

TRACE는 남아 있는 유효 점에서 계속 이동하며 경계에서는 `TRACE: invalid region`을 표시합니다.
신뢰할 수 있는 다음 구간이 제공되면 가장 가까운 다음 유효 점으로 이동합니다. X-CAL/ROOT/MAX/MIN/ICPT는
계산된 유효 구간에서만 찾고 x순으로 결과를 표시하며, 빈 구간을 가로질러 근·극값을 만들지 않습니다.
Y-CAL과 TRACE x=는 요청한 x까지 해당 IC에서 적분할 수 있을 때만 값을 반환합니다.

**현재 RK4는 각 IC에서 양방향으로 성공한 prefix를 보존합니다.** 실패 지점 너머의 같은 해를
새 초기조건 없이 재시작하지 않습니다. 반대 방향과 별도 IC는 계속 사용할 수 있지만 별도 IC는 별도 곡선입니다.
합성 valid/gap/valid 테스트는 신뢰 가능한 양쪽 값이 주어졌을 때의 소비 경로를 검증하며,
일반 ODE가 특이점을 통과했다는 뜻이 아닙니다. 결과 없음은 계산되지 않은 영역의 해 부재를 보장하지 않습니다.
자세한 경계는 [Numerical validity audit](NUMERICAL_VALIDITY_AUDIT.md)에 있습니다.

## Table·STAT

Table은 선택한 IC/방향에 대해 7행씩 표시합니다. 좌/우로 선택된 열을 스크롤하고 UP/DOWN으로 page를 이동합니다. TOP은 첫 data, BTM은 실제 적분 종료가 들어 있는 마지막 page, DIR은 ±x, IC+는 family를 바꿉니다.

STAT은 현재 Table의 IC·방향과 Output의 `L` 열만 `DIFFSTAT00.csv`부터 첫 빈 파일에 기록합니다. 첫 label 행은 각 cell이 `'`로 시작하므로 fx-CG50 List Editor가 공식 규칙에 따라 무시합니다. 계산기 STAT의 List Editor에서 `F6 → F6 → CSV → LOAD → FILE`을 선택하고 생성 파일을 엽니다. 공개 gint/fxlibc에는 Main Memory List 생성 API가 없어 비공개 메모리나 syscall은 사용하지 않습니다.

```csv
'x,'y,'y'
0,1,0
```

List Editor CSV는 최대 26열×999행입니다. label 행을 포함하므로 export는 998 data row로 제한하며 초과하면 파일을 삭제하고 Step을 늘리라고 안내합니다. RK4 callback이 끝난 뒤 고정 Table page를 formatting하고, 파일 create/append/close를 설치된 gint가 요구하는 OS-world transaction으로 수행합니다. 오류 파일은 handle이 정상적으로 닫힌 경우에만 안전하게 제거합니다. 이 경로는 **HARDWARE RETEST REQUIRED**입니다.

## Recall과 저장

**[USER REQUESTED ADAPTATION]** Main F6 SAVE는 현재 식·IC·설정·Recall을 명시적으로
저장합니다. 아직 equation을 선택하지 않았다면 No session을 표시합니다. 일반 편집 확정,
NEXT/PREV/EXIT에서는 RAM만 유지하며 자동으로 session 파일을 쓰지 않습니다.

Main F5 RCL에서 **Last calculation (RAM)** 또는 **Load saved session**을 선택합니다.
Main의 짧은 설명 `Recall / load session`은 이 두 기능을 함께 나타냅니다.
Last calculation은 기존 의미를 유지해 마지막 GRAPH 계산의 식·IC·Output·상수를 복원하고
현재 app-wide Solver/V-Window 설정은 유지합니다. Load saved session은 확인 후 저장된
current/recall과 설정을 모두 복원합니다. 진짜 새 실행은 defaults이며, 기존 파일은 삭제하지
않습니다. MENU 왕복으로 같은 실행이 재개될 때는 이 초기화를 다시 하지 않습니다.

계산기 저장 root에 `DIFFEQ0.dat`와 `DIFFEQ1.dat`를 번갈아 씁니다. 한 slot이 잘렸거나 검사가 실패하면 다른 정상 slot을 사용합니다. 두 slot은 앱 전용 형식으로 magic/version/size/checksum과 필드 범위를 검증합니다. 저장은 색상을 포함한 v4 layout을 작은 chunk로 streaming하며, 기존 v3는 같은 target ABI에서 읽어 기본 색상을 부여합니다. 잘못된 색상 번호는 곡선별 기본값으로 복구합니다. 지원하지 않는 버전이나 손상 slot은 안전하게 건너뜁니다. 모든 Fugue 호출을 `gint_world_switch()` 경계 안에서 끝냅니다. raw ABI 형식이므로 host 테스트의 `.dat`는 계산기로 이식할 수 없고 향후 형식 변경 시 거부될 수 있습니다. `DIFFSTATnn.csv`는 STAT/PC 교환용입니다. 두 slot이 있어도 실제 전원 차단 시 내구성은 하드웨어 검증 전입니다.

## 바로 확인할 예제

1. **1st → Separable**의 기본값 f=1, g=y²−1, IC (0,0)은 감소하는 곡선을 그립니다. IC 화면의 ADD로 (0,1)을 추가하면 수평 y=1도 함께 표시됩니다. 각 set은 독립된 초기값 문제입니다.
2. **2nd**의 기본값 f=0, g=1, h=0, 초기 y=1, y'=0은 cos(x)와 −sin(x)입니다.
3. **SYS → 2**의 기본값 y1'=y2, y2'=−y1, (1,0)에서 NEXT → NEXT → GRAPH → OPTN → phase 전환 → ZOOM → AUTO로 oscillator phase를 봅니다.
4. **N-th → 3**에서 `sin(x)-y1-y2`를 입력하고 OPTN → Convert to system를 실행하면 `y1'=y2`, `y2'=y3`, `y3'=sin(x)-y2-y3`로 변환됩니다.

오차 확인 예제: 일반 1차 `y`, y(0)=1, x=1에서 h=.1의 RK4 값은 약 2.718279744입니다. h=.05, .025로 줄이면 host global error가 각각 약 1/15.35, 1/15.67로 줄었습니다. 목표 하드웨어 수학 라이브러리에서의 실제 값은 따로 검증해야 합니다.
