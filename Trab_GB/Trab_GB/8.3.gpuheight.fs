#version 410 core

in float Height;

out vec4 FragColor;

void main()
{

    float h = (Height)/64.0f / 8;

    if(h <= 0.22f) // azul agua
    {
        FragColor = vec4(0, 0, 1 * h, 0.2);
    }
    else if(h <= 0.25f) // areia
    {
        FragColor = vec4(1 * h, 1 * h, 0, 1.0);
    }
    else if(h <= 0.40f) // floresta
    {
        FragColor = vec4(0, h, 0, 1.0 * h);
    }
    else if(h <= 0.50f) // montanhas marrom
    {
        FragColor = vec4(0.7 * h, 0.7 * h, 0.2 * h, 1.0);
    }
   else if(h <= 0.55f) // montanhas cinza
    {
        FragColor = vec4(0.7 * h, 0.7 * h, 0.7 * h, 1.0);
    }
    else if(h <= 0.60f) // neve
    {
        FragColor = vec4(h, h, h, 1.0);
    }
}