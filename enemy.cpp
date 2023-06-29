//=============================================================================
//
// エネミー処理 [enemy.cpp]
// Author : 
//
//=============================================================================
#include "enemy.h"
#include "player.h"
#include "input.h"
#include "bg.h"
#include "collision.h"
#include "score.h"
#include "fade.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(100.0f)//(800/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(100.0f)//(800/2)	// 
#define TEXTURE_MAX					(3)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/enemy00.png",
	"data/TEXTURE/runningman000.png",
	"data/TEXTURE/runningman002.png",
};


static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static ENEMY	g_Enemy[ENEMY_MAX];	// エネミー構造体


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// エネミー構造体の初期化
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		g_Enemy[i].use = TRUE;
		g_Enemy[i].pos = XMFLOAT3(100.0f * i, 100.0f, 0.0f);	// 中心点から表示
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);		// 回転率
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);		// 拡大縮小
		g_Enemy[i].w = TEXTURE_WIDTH;
		g_Enemy[i].h = TEXTURE_HEIGHT;
		g_Enemy[i].texNo = 0;

		g_Enemy[i].countAnim = 0;
		g_Enemy[i].patternAnim = 0;

		g_Enemy[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		g_Enemy[i].right = 1;								//右へ移動させる

	}


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	int count = 0;
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		// 生きてるエネミーだけ処理をする
		if (g_Enemy[i].use == TRUE)
		{
			count++;
			g_Enemy[i].rot.z += 0.1f;

			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Enemy[i].pos;

			// アニメーション  
			g_Enemy[i].countAnim += 1.0f;
			if (g_Enemy[i].countAnim > ANIM_WAIT)
			{
				g_Enemy[i].countAnim = 0.0f;
				// パターンの切り替え
				g_Enemy[i].patternAnim = (g_Enemy[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}

			// 移動処理
			if (i == 0)
			{
				PLAYER* player = GetPlayer();

				// 一般的なホーミング
				XMVECTOR epos = XMLoadFloat3(&g_Enemy[i].pos);
				XMVECTOR vec = XMLoadFloat3(&player[0].pos) - epos;
				float angle = atan2f(vec.m128_f32[1], vec.m128_f32[0]);
				float speed = 2.0f;

				g_Enemy[i].pos.x += cosf(angle) * speed;
				g_Enemy[i].pos.y += sinf(angle) * speed;


				//// なんちゃってホーミング処理
				//float vx = (player[0].pos.x - g_Enemy[0].pos.x) * 0.01f;
				//float vy = (player[0].pos.y - g_Enemy[0].pos.y) * 0.01f;
				//g_Enemy[i].pos.x += vx;
				//g_Enemy[i].pos.y += vy;

				//// DX11用ホーミング
				//XMVECTOR epos = XMLoadFloat3(&g_Enemy[i].pos);
				//XMVECTOR vec = (XMLoadFloat3(&player[0].pos) - epos) * 0.01f;
				//epos += vec;
				//XMStoreFloat3(&g_Enemy[i].pos, epos);


			}
			else
			{
				// 左右に往復するプログラム

				float speed = g_Enemy[i].move.x;

				if (g_Enemy[i].right == 1)
				{
					//右端に行くまでは右に移動させる
					g_Enemy[i].pos.x += speed; //右に動く
				}
				else
				{
					//左端に行くまでは左に移動させる
					g_Enemy[i].pos.x -= speed; //左に動く
				}

				// MAP外チェック
				BG* bg = GetBG();

				if (g_Enemy[i].pos.x < 0.0f)
				{
					g_Enemy[i].pos.x = 0.0f;
					//g_Enemy[i].move.x = 0 - g_Enemy[i].move.x;
					g_Enemy[i].right = 1;		//一番左についたらフラグを右に変える

				}

				if (g_Enemy[i].pos.x > bg->w)
				{
					g_Enemy[i].pos.x = bg->w;
					//g_Enemy[i].move.x = 0 - g_Enemy[i].move.x;
					g_Enemy[i].right = 0;		//一番右についたらフラグを左に変える
				}

				if (g_Enemy[i].pos.y < 0.0f)
				{
					g_Enemy[i].pos.y = 0.0f;
				}

				if (g_Enemy[i].pos.y > bg->h)
				{
					g_Enemy[i].pos.y = bg->h;
				}
			}
			// 移動が終わったらプレイヤーとの当たり判定

			//for (int x = 0; x < ENEMY_MAX; x++)
			//{
			//	PLAYER* player = GetPlayer();

			//	// 生きてるプレイヤーだけ判定を行う
			//	if (player[i].use == TRUE)
			//	{
			//		// BBの判定を行う
			//		if (CollisionBB(g_Enemy[i].pos, g_Enemy[i].w, g_Enemy[i].h, player[x].pos, player[x].w, player[x].h) == TRUE)
			//		{
			//			// プレイヤーと接触したら消える
			//			player[i].use = FALSE;
			//			AddScore(10);
			//		}

			//	}

			//}



			// バレット処理
			if (GetKeyboardTrigger(DIK_SPACE))
			{


			}
			if (IsButtonTriggered(0, BUTTON_B))
			{


			}
		}
	}




	if (count == 0)
	{
		//SetMode(MODE_RESULT);
		SetFade(FADE_OUT, MODE_RESULT);

	}

#ifdef _DEBUG	// デバッグ情報を表示する


#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == TRUE)		// このエネミーが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Enemy[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float ex = g_Enemy[i].pos.x;	// エネミーの表示位置X
			float ey = g_Enemy[i].pos.y;	// エネミーの表示位置Y
			float ew = g_Enemy[i].w;		// エネミーの表示幅
			float eh = g_Enemy[i].h;		// エネミーの表示高さ

			// アニメーション用
			//float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
			//float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
			//float tx = (float)(g_Enemy[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
			//float ty = (float)(g_Enemy[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標

			float tw = 1.0f;	// テクスチャの幅
			float th = 1.0f;	// テクスチャの高さ
			float tx = 0.0f;	// テクスチャの左上X座標
			float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, ex, ey, ew, eh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Enemy[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}


}


//=============================================================================
// Enemy構造体の先頭アドレスを取得
//=============================================================================
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0];
}




