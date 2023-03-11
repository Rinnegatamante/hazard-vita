
uniform sampler2D _texture;

void main(
	float2 texCoord : TEXCOORD0,
	float4 colour : COLOR,
	float4 out gl_FragColor : COLOR
) {
	float4 tex = tex2D(_texture, texCoord);
	gl_FragColor = tex * colour;
}
