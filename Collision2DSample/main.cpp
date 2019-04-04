#include <DxLib.h>
#include <boost/operators.hpp>
#include <math.h>

struct Vector2
	: boost::equality_comparable<Vector2>
	, boost::addable<Vector2>
	, boost::subtractable<Vector2>
	, boost::multipliable<Vector2, double>
	, boost::dividable<Vector2, double> {
	double x;
	double y;

	Vector2() = default;
	Vector2(const Vector2 &) = default;
	Vector2(Vector2&&) = default;
	constexpr Vector2(double x, double y) : x(x), y(y) {}
	~Vector2() = default;
	Vector2& operator=(const Vector2&) = default;
	Vector2& operator=(Vector2&&) = default;

	bool operator==(const Vector2 &rhs) const { return x == rhs.x && y == rhs.y; }
	Vector2& operator+=(const Vector2 &rhs) { x += rhs.x; y += rhs.y; return *this; }
	Vector2& operator-=(const Vector2 &rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	Vector2& operator*=(const double &rhs) { x *= rhs; y *= rhs; return *this; }
	Vector2& operator/=(const double &rhs) { x /= rhs; y /= rhs; return *this; }
};

double abs(Vector2 v)
{
	return sqrt(v.x*v.x + v.y*v.y);
}

constexpr double dot(Vector2 lhs, Vector2 rhs)
{
	return lhs.x*rhs.x + lhs.y*rhs.y;
}

constexpr double cross(Vector2 lhs, Vector2 rhs)
{
	return lhs.x*rhs.y - rhs.x*lhs.y;
}

constexpr bool collidesRectangles(Vector2 r1, Vector2 s1, Vector2 r2, Vector2 s2)
{
	return r2.x < r1.x+s1.x && r1.x < r2.x+s2.x && r2.y < r1.y+s1.y && r1.y < r2.y+s2.y;
}

Vector2 pr, ppr;
constexpr double GravitationalAcceleration = 0.6;
constexpr double pv = 4;

int graphic, playerGraphic;
int key = 0, pkey = 0, tkey = 0;

constexpr Vector2 ChipSize{32, 32};
constexpr Vector2 ScreenSize{640, 480};
constexpr Vector2 pcs{24, 30}, pcr{(ChipSize.x - pcs.x) / 2, ChipSize.y - pcs.y};

Vector2 br{300, 200}, bs{100, 100};

bool CollidesParallelogramLine(
	double x1, double y1, double x2, double y2, double pw, double lx, double ly, double lw)
{
	return y1 <= ly && ly <= y2 &&
		(y2-y1)*(lx-x1-pw) < (x2-x1)*(ly-y1) && (x2-x1)*(ly-y1) <= (y2-y1)*(lx-x1+lw);
}

Vector2 adjustedLocation(Vector2 or, Vector2 por, Vector2 br)
{
	Vector2 dp = or - por;
	double c = dp.x >= 0 && dp.y >= 0 || dp.x < 0 && dp.y < 0 ? cross(br-por, dp) : cross(dp, br-por);
	return c >= 0 ? Vector2{br.x, or.y} : Vector2{or.x, br.y};
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 画面モードのセット
	SetGraphMode(static_cast<int>(ScreenSize.x), static_cast<int>(ScreenSize.y), 32);
	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1) return -1;

	// 描画先画面を裏画面にセット
	SetDrawScreen(DX_SCREEN_BACK);

	graphic = LoadGraph("graphic.png");
	playerGraphic = DerivationGraph(0, 0, 32, 32, graphic);

	// ループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
		ClearDrawScreen();
		DrawBox(0, 0, static_cast<int>(ScreenSize.x), static_cast<int>(ScreenSize.y), GetColor(64, 64, 192), TRUE);
		pkey = key;
		key = GetJoypadInputState(DX_INPUT_KEY_PAD1);
		tkey = ~pkey & key;

		ppr = pr;
		if (key & PAD_INPUT_LEFT) pr.x -= pv;
		if (key & PAD_INPUT_RIGHT) pr.x += pv;
		if (key & PAD_INPUT_UP) pr.y -= pv;
		if (key & PAD_INPUT_DOWN) pr.y += pv;

		{
			Vector2 hr{pr+pcr}, hs{pcs};
			if (collidesRectangles(hr, hs, br, bs)) {
				Vector2 phr{ppr+pcr}, dhr = hr - phr;
				Vector2 ho = Vector2(dhr.x >= 0 ? hs.x : 0, dhr.y >= 0 ? hs.y : 0);
				Vector2 bo = Vector2(dhr.x >= 0 ? 0 : bs.x, dhr.y >= 0 ? 0 : bs.y);
				Vector2 c = hr + ho, pc = phr + ho;
				Vector2 ac = adjustedLocation(c, pc, br + bo);
				pr = ac - ho - pcr;
			}
		}
		{
			Vector2 hr{pr+pcr}, hs{pcs};
			DrawGraph(static_cast<int>(pr.x), static_cast<int>(pr.y), playerGraphic, TRUE);
			Vector2 hbrr = pr + pcr + hs;
			DrawBox(static_cast<int>(hr.x), static_cast<int>(hr.y),
				static_cast<int>(hbrr.x), static_cast<int>(hbrr.y),
				GetColor(0, 192, 0), FALSE);
		}
		Vector2 bbrr = br+bs;
		DrawBox(static_cast<int>(br.x), static_cast<int>(br.y), static_cast<int>(bbrr.x), static_cast<int>(bbrr.y),
			GetColor(255, 0, 0), FALSE);

		ScreenFlip();
	}

	DxLib_End();

	return 0;
}