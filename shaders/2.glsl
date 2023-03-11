

void main(
	 float2 in_Vertex,
	 float4 in_Colour,
	 float2 in_TexCoord,
	 float in_Rotation,
	 float2 in_Velocity,
	 float in_specularPower,
	 float in_blurPower,
	 float3  in_replaceColour,
	 float4 out colour : COLOR,
	 float4 out  texCoords : TEXCOORD0,
	 float4 out texCoords2 : TEXCOORD1,
	 float4 out texCoords3 : TEXCOORD2,
	 float4 out gl_Position : POSITION
) {
	// pass colour and text coord to fragment shader
	colour = in_Colour;
	
	texCoords.xy = in_TexCoord;
	texCoords.zw = in_Velocity;
	
	texCoords2.xyz = in_replaceColour;
	texCoords2.w = in_Rotation;
	
	texCoords3.xy = in_Vertex;
	texCoords3.z = in_specularPower;
	texCoords3.w = in_blurPower;

	// set vertex position
	gl_Position = float4(in_Vertex, 0.0, 1.0);
}
