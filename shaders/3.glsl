
uniform sampler2D _texture;
uniform sampler2D normalMap;

// lights

//values used for shading algorithm...
uniform int  numLights;
uniform float3  lightPos[50];		//light position, normalized
uniform float4  lightColour[50];	 //light RGBA -- alpha is intensity
uniform float lightSize[50];  

uniform float4 ambientColour;	//ambient RGBA -- alpha is intensity 
uniform float screenScaleY;	// so we can adjust the light to the correct aspect ratio

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
float3 rgb2hsv(float3 c)
{
	float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	float4 p = c.g < c.b ? float4(c.bg, K.wz) : float4(c.gb, K.xy);
	float4 q = c.r < p.x ? float4(p.xyw, c.r) : float4(c.r, p.yzx);

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
float3 hsv2rgb(float3 c)
{
	float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
float4 doReplaceColour(float4 c, float3 replaceColour)
{
	if(c.a > 0.0)
	{
		if(replaceColour.x != 0.0 || replaceColour.y != 0.0 || replaceColour.z != 0.0)
		{
			float4 h;
			h.a = c.a;
			h.rgb = rgb2hsv(c.rgb);

			if(h.y > 0.3)   // if saturation is over threshold 
			{
				if(h.x > 0.95 || h.x <= 0.1)	// if hue is red(ish)
				{
					h.x += replaceColour.x;
					h.y *= replaceColour.y;
					h.z *= replaceColour.z;
					c.rgb = hsv2rgb(h.rgb);
				}
			}
		}
	}

	return c;
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
float2 rotate2D(float2 v, float s, float c)
{
	float2 r;
	r.x = v.x*c - v.y*s;
	r.y = v.x*s + v.y*c;

	return r;
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
void motionBlur(
	float3 ambient,
	float s,
	float c,
	float4 inout gl_FragColor,
	float4 inout bluredNormal,
	float2 inout gVelocity,
	float blurPower,
	float2 texCoord,
	float4 colour,
	float3 replaceColour
) {
	float4 diffuseColour;
	float3 finalColour;
	float2 blurTexCoord;
	float steps;
	float n;

	// adjust velocity for screen aspect ratio
	gVelocity.y *= screenScaleY;
	gVelocity *= blurPower * 200.0;

	if(length(gVelocity) > 10.0)
	{
		gVelocity = normalize(gVelocity);
		gVelocity *= 10.0;
	}

	steps = length(gVelocity) * 1.5;	 // blur factor!
	steps = min(15.0, steps);

	if(steps <= 1.0)
	{
		bluredNormal = tex2D(normalMap, texCoord);	  // Blurred normal is just normal map

		diffuseColour = tex2D(_texture, texCoord);

		// replace colour test - works quite well
		diffuseColour = doReplaceColour(diffuseColour, replaceColour);

		finalColour.rgb = ambient.rgb * diffuseColour.rgb;
		gl_FragColor = colour * float4(finalColour, diffuseColour.a);
		return;
	}

	// rotate light for sprite rotation
	gVelocity = rotate2D(gVelocity, s, c);

	gVelocity.y *= -1.0;	 // flip Y
	gVelocity /= 4000.0;

	blurTexCoord = texCoord;
	blurTexCoord -= gVelocity * steps/2.0;

	steps = floor(steps);
	for(n=0.0; n<steps; n+=1.0)
	{
		blurTexCoord += gVelocity;
		diffuseColour = tex2D(_texture, blurTexCoord);

		// replace colour test - works quite well
		diffuseColour = doReplaceColour(diffuseColour, replaceColour);

		finalColour.rgb = ambient.rgb * diffuseColour.rgb;
		gl_FragColor += colour * float4(finalColour, diffuseColour.a);

		bluredNormal += tex2D(normalMap, blurTexCoord);
	}

	gl_FragColor /= steps;
	bluredNormal /= steps;
//	gl_FragColor.a *= 2.0;
//	gl_FragColor.a = min(1.0, gl_FragColor.a);

//	gl_FragColor.g = steps / 10.0;
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
void doLights(float3 N, float s, float c, float2 gPixelPos, float4 inout gl_FragColor, float specularPower, float4 colour)
{
	int i;
	float dist;
	float specDist;

	float lightsize;
	float specSize;
	float f, ff;

	float3 diffuse;
	float specularReflection;
	float3 reflectDir;
	float3 finalColour;

	float3 viewDir = float3(0.0, 0.0, 1.0);
	float4 diffuseColour = gl_FragColor;// texture2D(texture, texCoord); // Light using the blurred pixel

	float3 lightPosTmp;

	// loop through each light
	for(i=0; i<numLights; i++)
	{
		// adjust these for aspect ratio
		lightPosTmp = lightPos[i];
		lightPosTmp.y *= screenScaleY;

		// calc light distance and specular distance
		dist = length(lightPosTmp.xy - gPixelPos);
		specDist = dist;

		lightsize = lightSize[i] * length(lightColour[i].rgb);
		specSize = lightsize + 0.2;

		// calc specular distance
		if(specDist < specSize)
		{
			specDist = specSize - specDist;
			specDist *= 1.0 / specSize;
		}
		else
			specDist = 0.0;

		// bug out if light doesn't get this far
		if(specDist == 0.0)
			continue;

		// calc light distance
		if(dist < lightsize)
		{
			if(dist > lightsize * 0.3)
			{
				// add falloff for last 30% of radius
				dist = lightsize - dist;
				dist *= 1.0 / (lightsize * (1.0 - 0.3));
			}
			else
				dist = 1.0;
		}
		else
			dist = 0.0;


		// ===== apply lighting
		ff = gl_FragColor.a;

		// make light pos relative to normal map
		lightPosTmp.xy -= gPixelPos.xy;
		lightPosTmp = normalize(lightPosTmp);

		// rotate light for sprite rotation
		lightPosTmp.xy = rotate2D(lightPosTmp.xy, s, c);

		//Pre-multiply light colour with intensity
		//Then perform "N dot L" to determine our diffuse term
		f = max(dot(N, lightPosTmp), 0.0);
		diffuse = (lightColour[i].rgb * lightColour[i].a * 3.0) * f;

		// calc specular highlight
		if(dot(N, lightPosTmp) < 0.0) // light source on the wrong side?
		{
			specularReflection = 0.0; // float3(0.0, 0.0, 0.0); // no specular reflection
		}
		else // light source on the right side
		{
			reflectDir = reflect(-lightPosTmp, N);
			specularReflection = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
		}

		// calc final colour
		finalColour = diffuseColour.rgb * diffuse * dist * ff;
		finalColour += specularReflection * specDist * length(lightColour[i].rgb) * specularPower;
		gl_FragColor += colour * float4(finalColour, diffuseColour.a);
		gl_FragColor.a = ff;
	}
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
void main(
	float4 colour : COLOR,
	float4 texCoords : TEXCOORD0,
	float4 texCoords2 : TEXCOORD1,
	float4 texCoords3 : TEXCOORD2,
	float4 out gl_FragColor : COLOR
) {
	float2 texCoord = texCoords.xy;
	float2 velocity = texCoords.zw;
	
	float3 replaceColour = texCoords2.xyz;
	float rotation = texCoords2.w;
	
	float2 pixelPos = texCoords3.xy;
	float specularPower = texCoords3.z;
	float blurPower = texCoords3.w;
	
	float4 diffuseColour;

	// fast draw on mobile if no lights and no blur
	if(numLights == 0 && blurPower == 0.0)
	{
		// write normal pixel
		diffuseColour = tex2D(_texture, texCoord);

		// replace colour test - works quite well
		if(diffuseColour.a > 0.0)
		{
			if(replaceColour.x != 0.0 || replaceColour.y != 0.0 || replaceColour.z != 0.0)
				diffuseColour = doReplaceColour(diffuseColour, replaceColour);
		}
		gl_FragColor = colour * diffuseColour;
		return;
	}


	gl_FragColor = float4(0.0, 0.0, 0.0, 0.0);
	float4 bluredNormal = float4(0.0, 0.0, 0.0, 0.0);

	// init globals
	float2 gVelocity = velocity;
	float2 gPixelPos = pixelPos;

	// ----- Normal Map
	float3 finalColour;
	float3 ambient;

	// for rotation later
	float s = sin(rotation);
	float c = cos(rotation);

	// calc ambient colour
	ambient = ambientColour.rgb * ambientColour.a * 3.0;

	// apply motion blur
	if(gVelocity.x * blurPower != 0.0 || gVelocity.y * blurPower != 0.0)
	{
		motionBlur(ambient, s, c, gl_FragColor, bluredNormal, gVelocity, blurPower, texCoord, colour, replaceColour);
	}
	else
	{
		// write normal pixel
		diffuseColour = tex2D(_texture, texCoord);

		// replace colour test - works quite well
		diffuseColour = doReplaceColour(diffuseColour, replaceColour);

		finalColour.rgb = ambient.rgb * diffuseColour.rgb;
		gl_FragColor = colour * float4(finalColour, diffuseColour.a);

		bluredNormal = tex2D(normalMap, texCoord);	  // Blurred normal is just normal map
	}

	// early out for black shadows
	if(gl_FragColor.r == 0.0 && gl_FragColor.g == 0.0 && gl_FragColor.b == 0.0 && gl_FragColor.a < 0.5)
		return;

	// early out 
	if(gl_FragColor.a == 0.0)
		return;

	// adjust pixel pos for screen aspect ratio
	gPixelPos.y *= screenScaleY;

	//RGB of our normal map
	float3 normalColour =  bluredNormal.rgb;// texture2D(normalMap, texCoord).rgb;

	//normalize our floattors
	float3 N = normalize(normalColour * 2.0 - 1.0);

	// now apply lighting
	if(numLights > 0)
		doLights(N, s, c, gPixelPos, gl_FragColor, specularPower, colour);
}

