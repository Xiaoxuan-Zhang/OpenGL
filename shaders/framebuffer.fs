#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
const float offset = 1.0/768;

void main() {
  vec2 offsets[9] = vec2[](
      vec2(-offset,  offset), // top-left
      vec2( 0.0f,    offset), // top-center
      vec2( offset,  offset), // top-right
      vec2(-offset,  0.0f),   // center-left
      vec2( 0.0f,    0.0f),   // center-center
      vec2( offset,  0.0f),   // center-right
      vec2(-offset, -offset), // bottom-left
      vec2( 0.0f,   -offset), // bottom-center
      vec2( offset, -offset)  // bottom-right
  );
  //Sharpen
  float kernel1[9] = float[](
    -1.0, -1.0, -1.0,
    -1.0, 9.0f, -1.0,
    -1.0, -1.0, -1.0
  );
  //blur
  float kernel2[9] = float[](
    1.0/16.0, 2.0/16.0, 1.0/16.0,
    2.0/16.0, 4.0/16.0, 2.0/16.0,
    1.0/16.0, 2.0/16.0, 1.0/16.0
  );
  //edge detection
  float kernel3[9] = float[](
    1.0, 1.0, 1.0,
    1.0, -8.0f, 1.0,
    1.0, 1.0, 1.0
  );

  vec3 colors = vec3(0.0);
  for(int i = 0; i < 9; i++)
  {
      colors += vec3(texture(screenTexture, TexCoords + offsets[i])) * kernel1[i];
  }
  /* using kernel */
  //FragColor = vec4(colors, 1.0);

  vec4 texColor = texture(screenTexture, TexCoords);
  /* Reverse color */
  //FragColor = vec4(1.0 - vec3(texColor), 1.0);
  /* Gray scale */
  float average0 = 0.2126 * texColor.r + 0.7152 * texColor.g + 0.0722 * texColor.b;
  float average1 = ( texColor.r + texColor.g + texColor.b)/3.0;
  float average = mix(average0, average1, abs(sin(time)));
  //FragColor = vec4(average, average, average, 1.0);

  /* nothing special */
  FragColor = vec4(texColor.rgb, 1.0);
}
