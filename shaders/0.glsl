
void main(
	float2 in_Vertex,
	float2 in_TexCoord,
	float4 in_Colour,
	float4 out colour : COLOR,
	float2 out texCoord : TEXCOORD0,
	float4 out gl_Position : POSITION
) {
	// pass colour and text coord to fragment shader
	colour = in_Colour;
	texCoord = in_TexCoord;

	// set vertex position
	gl_Position = float4(in_Vertex, 0.0, 1.0);
}
