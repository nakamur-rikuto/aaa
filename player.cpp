//=============================================================================
//
// �v���C���[���� [player.cpp]
// Author : 
//
//=============================================================================
#include "enemy.h"
#include "player.h"
#include "input.h"
#include "bg.h"
#include "collision.h"
#include "bullet.h"
#include "score.h"
#include "sound.h"
#include "fade.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(200.0f)//(800/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(200.0f)//(800/2)	// 
#define TEXTURE_MAX					(3)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(8)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
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
	"data/TEXTURE/runningman000.png",
	"data/TEXTURE/runningman002.png",
};


static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static PLAYER	g_Player[PLAYER_MAX];	// �v���C���[�\����


//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayer(void)
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


	// �v���C���[�\���̂̏�����
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_Player[i].use = TRUE;
		g_Player[i].pos = XMFLOAT3(400.0f, 400.0f, 0.0f);	// ���S�_����\��
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);		// ��]��
		g_Player[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);		// �g��k��
		g_Player[i].w = TEXTURE_WIDTH;
		g_Player[i].h = TEXTURE_HEIGHT;
		g_Player[i].texNo = 0;

		g_Player[i].countAnim = 0;
		g_Player[i].patternAnim = 0;

		g_Player[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

	}


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPlayer(void)
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
void UpdatePlayer(void)
{
	int count = 0;
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		// �����Ă�v���C���[��������������
		if (g_Player[i].use == TRUE)
		{
			count++;
			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Player[i].pos;

			// �A�j���[�V����  
			g_Player[i].countAnim += 1.0f;
			if (g_Player[i].countAnim > ANIM_WAIT)
			{
				g_Player[i].countAnim = 0.0f;
				// �p�^�[���̐؂�ւ�
				g_Player[i].patternAnim = (g_Player[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}

			// �L�[���͂ňړ� 
			{
				float speed = g_Player[i].move.x;

				if (GetKeyboardPress(DIK_C))
				{
					speed *= 4;
				}


				if (GetKeyboardPress(DIK_DOWN))
				{
					g_Player[i].pos.y += speed;
				}
				else if (GetKeyboardPress(DIK_UP))
				{
					g_Player[i].pos.y -= speed;
				}

				if (GetKeyboardPress(DIK_RIGHT))
				{
					g_Player[i].pos.x += speed;
				}
				else if (GetKeyboardPress(DIK_LEFT))
				{
					g_Player[i].pos.x -= speed;
				}

				// �Q�[���p�b�h�ňړ�����
				if (IsButtonPressed(0, BUTTON_DOWN))
				{
					g_Player[i].pos.y += speed;
				}
				else if (IsButtonPressed(0, BUTTON_UP))
				{
					g_Player[i].pos.y -= speed;
				}

				if (IsButtonPressed(0, BUTTON_RIGHT))
				{
					g_Player[i].pos.x += speed;
				}
				else if (IsButtonPressed(0, BUTTON_LEFT))
				{
					g_Player[i].pos.x -= speed;
				}

				// MAP�O�`�F�b�N
				BG* bg = GetBG();

				if (g_Player[i].pos.x < 0.0f)
				{
					g_Player[i].pos.x = 0.0f;
				}

				if (g_Player[i].pos.x > bg->w)
				{
					g_Player[i].pos.x = bg->w;
				}

				if (g_Player[i].pos.y < 0.0f)
				{
					g_Player[i].pos.y = 0.0f;
				}

				if (g_Player[i].pos.y > bg->h)
				{
					g_Player[i].pos.y = bg->h;
				}


				// �ړ����I�������G�l�~�[�Ƃ̓����蔻��
				ENEMY* enemy = GetEnemy();

				for (int x = 0; x < ENEMY_MAX; x++)
				{
					// �����Ă�G�l�~�[����������s��
					if (enemy[x].use == TRUE)
					{
						// BB�̔�����s��
						if (CollisionBB(g_Player[i].pos, g_Player[i].w, g_Player[i].h, enemy[x].pos, enemy[x].w, enemy[x].h) == TRUE)
						{
							// �G�l�~�[�ƐڐG�����������
							enemy[x].use = FALSE;
							AddScore(10);
						}

					}

				}


				// �o���b�g����
				if (GetKeyboardTrigger(DIK_SPACE))
				{
					XMFLOAT3 pos = g_Player[i].pos;
					pos.y -= 100.0f;
					SetBullet(pos);
					pos.x -= 100.0f;

					SetBullet(pos);
					pos.x += 200.0f;
					SetBullet(pos);
					PlaySound(SOUND_LABEL_SE_shot000);


				}
				if (IsButtonTriggered(0, BUTTON_B))
				{
					XMFLOAT3 pos = g_Player[i].pos;
					pos.y -= 100.0f;
					SetBullet(pos);

				}

			}
		}
	}




	if (count == 0)
	{
		SetMode(MODE_RESULT);
		SetFade(FADE_OUT, MODE_RESULT);

	}

#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawPlayer(void)
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

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Player[i].use == TRUE)		// ���̃v���C���[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Player[i].texNo]);

			//�v���C���[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Player[i].pos.x;	// �v���C���[�̕\���ʒuX
			float py = g_Player[i].pos.y;	// �v���C���[�̕\���ʒuY
			float pw = g_Player[i].w;		// �v���C���[�̕\����
			float ph = g_Player[i].h;		// �v���C���[�̕\������

			// �A�j���[�V�����p
			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// �e�N�X�`���̕�
			float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// �e�N�X�`���̍���
			float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// �e�N�X�`���̍���Y���W

			//float tw = 1.0f;	// �e�N�X�`���̕�
			//float th = 1.0f;	// �e�N�X�`���̍���
			//float tx = 0.0f;	// �e�N�X�`���̍���X���W
			//float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Player[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}


}


//=============================================================================
// Player�\���̂̐擪�A�h���X���擾
//=============================================================================
PLAYER* GetPlayer(void)
{
	return &g_Player[0];
}




