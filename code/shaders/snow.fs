#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture0;
uniform float alphaValue;

void main()
{
    vec4 color = texture(texture0, TexCoord);
    if (color.a < 0.1f)
		discard;
    FragColor = vec4(color.rgb, alphaValue);
}