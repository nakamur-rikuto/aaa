//=============================================================================
//
// �G�l�~�[���� [enemy.cpp]
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
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(100.0f)//(800/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(100.0f)//(800/2)	// 
#define TEXTURE_MAX					(3)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/enemy00.png",
	"data/TEXTURE/runningman000.png",
	"data/TEXTURE/runningman002.png",
};


static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static ENEMY	g_Enemy[ENEMY_MAX];	// �G�l�~�[�\����


//=============================================================================
// ����������
//=============================================================================
HRESULT InitEnemy(void)
{
	ID3D11Device *pDevice = GetDevice();

	//�e�N�X�`������
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


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �G�l�~�[�\���̂̏�����
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		g_Enemy[i].use = TRUE;
		g_Enemy[i].pos = XMFLOAT3(100.0f * i, 100.0f, 0.0f);	// ���S�_����\��
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);		// ��]��
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);		// �g��k��
		g_Enemy[i].w = TEXTURE_WIDTH;
		g_Enemy[i].h = TEXTURE_HEIGHT;
		g_Enemy[i].texNo = 0;

		g_Enemy[i].countAnim = 0;
		g_Enemy[i].patternAnim = 0;

		g_Enemy[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

		g_Enemy[i].right = 1;								//�E�ֈړ�������

	}


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
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
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
	int count = 0;
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		// �����Ă�G�l�~�[��������������
		if (g_Enemy[i].use == TRUE)
		{
			count++;
			g_Enemy[i].rot.z += 0.1f;

			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Enemy[i].pos;

			// �A�j���[�V����  
			g_Enemy[i].countAnim += 1.0f;
			if (g_Enemy[i].countAnim > ANIM_WAIT)
			{
				g_Enemy[i].countAnim = 0.0f;
				// �p�^�[���̐؂�ւ�
				g_Enemy[i].patternAnim = (g_Enemy[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}

			// �ړ�����
			if (i == 0)
			{
				PLAYER* player = GetPlayer();

				// ��ʓI�ȃz�[�~���O
				XMVECTOR epos = XMLoadFloat3(&g_Enemy[i].pos);
				XMVECTOR vec = XMLoadFloat3(&player[0].pos) - epos;
				float angle = atan2f(vec.m128_f32[1], vec.m128_f32[0]);
				float speed = 2.0f;

				g_Enemy[i].pos.x += cosf(angle) * speed;
				g_Enemy[i].pos.y += sinf(angle) * speed;


				//// �Ȃ񂿂���ăz�[�~���O����
				//float vx = (player[0].pos.x - g_Enemy[0].pos.x) * 0.01f;
				//float vy = (player[0].pos.y - g_Enemy[0].pos.y) * 0.01f;
				//g_Enemy[i].pos.x += vx;
				//g_Enemy[i].pos.y += vy;

				//// DX11�p�z�[�~���O
				//XMVECTOR epos = XMLoadFloat3(&g_Enemy[i].pos);
				//XMVECTOR vec = (XMLoadFloat3(&player[0].pos) - epos) * 0.01f;
				//epos += vec;
				//XMStoreFloat3(&g_Enemy[i].pos, epos);


			}
			else
			{
				// ���E�ɉ�������v���O����

				float speed = g_Enemy[i].move.x;

				if (g_Enemy[i].right == 1)
				{
					//�E�[�ɍs���܂ł͉E�Ɉړ�������
					g_Enemy[i].pos.x += speed; //�E�ɓ���
				}
				else
				{
					//���[�ɍs���܂ł͍��Ɉړ�������
					g_Enemy[i].pos.x -= speed; //���ɓ���
				}

				// MAP�O�`�F�b�N
				BG* bg = GetBG();

				if (g_Enemy[i].pos.x < 0.0f)
				{
					g_Enemy[i].pos.x = 0.0f;
					//g_Enemy[i].move.x = 0 - g_Enemy[i].move.x;
					g_Enemy[i].right = 1;		//��ԍ��ɂ�����t���O���E�ɕς���

				}

				if (g_Enemy[i].pos.x > bg->w)
				{
					g_Enemy[i].pos.x = bg->w;
					//g_Enemy[i].move.x = 0 - g_Enemy[i].move.x;
					g_Enemy[i].right = 0;		//��ԉE�ɂ�����t���O�����ɕς���
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
			// �ړ����I�������v���C���[�Ƃ̓����蔻��

			//for (int x = 0; x < ENEMY_MAX; x++)
			//{
			//	PLAYER* player = GetPlayer();

			//	// �����Ă�v���C���[����������s��
			//	if (player[i].use == TRUE)
			//	{
			//		// BB�̔�����s��
			//		if (CollisionBB(g_Enemy[i].pos, g_Enemy[i].w, g_Enemy[i].h, player[x].pos, player[x].w, player[x].h) == TRUE)
			//		{
			//			// �v���C���[�ƐڐG�����������
			//			player[i].use = FALSE;
			//			AddScore(10);
			//		}

			//	}

			//}



			// �o���b�g����
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

#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == TRUE)		// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Enemy[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float ex = g_Enemy[i].pos.x;	// �G�l�~�[�̕\���ʒuX
			float ey = g_Enemy[i].pos.y;	// �G�l�~�[�̕\���ʒuY
			float ew = g_Enemy[i].w;		// �G�l�~�[�̕\����
			float eh = g_Enemy[i].h;		// �G�l�~�[�̕\������

			// �A�j���[�V�����p
			//float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// �e�N�X�`���̕�
			//float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// �e�N�X�`���̍���
			//float tx = (float)(g_Enemy[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// �e�N�X�`���̍���X���W
			//float ty = (float)(g_Enemy[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// �e�N�X�`���̍���Y���W

			float tw = 1.0f;	// �e�N�X�`���̕�
			float th = 1.0f;	// �e�N�X�`���̍���
			float tx = 0.0f;	// �e�N�X�`���̍���X���W
			float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, ex, ey, ew, eh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Enemy[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}


}


//=============================================================================
// Enemy�\���̂̐擪�A�h���X���擾
//=============================================================================
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0];
}




