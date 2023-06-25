//�g�`�f�[�^
StructuredBuffer<float> amplitudes : register(t0);

cbuffer WaveData : register(b0)
{
	float4 lineColor;//���̐F
	uint frameCount;//�g�`�f�[�^�̃T���v������
	uint samplingRate;//�T���v�����O���[�g
	float tick;//�o�ߎ���
	float thickness;//���̌���
};

float isRange(float x, float a, float b)
{
	return step(min(a, b), x) * step(x, max(a, b));
}

float4 main(in float4 sv_position : SV_POSITION, in float2 texcoord : TEXCOORD) : SV_TARGET
{
	const float fineness = 50;

	//�T���v���J�n�ʒu���Z�o
	float samplingPositionBegin = samplingRate * tick;

	//�T���v���ʒu���Z�o
	float samplingPosition = samplingPositionBegin + samplingRate * trunc(texcoord.x * fineness);

	//�g�`�f�[�^���T���v�����O����-0.5 ~ 0.5�֕ϊ�����(1��)
	float amplitudeBegin = amplitudes[(uint) samplingPosition % frameCount] * 0.5 + 0.5;

	//1�T���v�����ړ�
	samplingPosition += samplingRate;

	//�g�`�f�[�^���T���v�����O����-0.5 ~ 0.5�֕ϊ�����(2��)
	float amplitudeEnd = amplitudes[(uint) samplingPosition % frameCount] * 0.5 + 0.5;

	float4 outColor = (float4) 0;

	//���݌��Ă���s�N�Z����2�̃T���v���Ԃ̂ǂ��Ɉʒu����̂��Z�o
	float amplitude = lerp(amplitudeBegin, amplitudeEnd, frac(texcoord.x * fineness));

	//���̕����Z�o
	float width = thickness * 0.5;

	//���݌��Ă���s�N�Z��������ɂ��邩����
	outColor = lerp(outColor, lineColor, isRange(texcoord.y, amplitude - width, amplitude + width));

	return outColor;
}
