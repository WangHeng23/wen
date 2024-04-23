#version 450

layout (location = 0) in vec2 fragCoord;

layout (binding = 0) uniform Input {
	vec3 iResolution;
	float iTime;
	float iTimeDelta;
	float iFrameRate;
	float iFrame;
	vec4 iMouse;
	vec4 iDate;
} ipt;

vec3 iResolution = ipt.iResolution;
float iTime = ipt.iTime;
float iTimeDelta = ipt.iTimeDelta;
float iFrameRate = ipt.iFrameRate;
float iFrame = ipt.iFrame;
vec4 iMouse = ipt.iMouse;
vec4 iDate = ipt.iDate;

layout(location = 0) out vec4 outColor;

void mainImage(out vec4 fragColor, in vec2 fragCoord);

void main() {
	mainImage(outColor, fragCoord);
}

#define steps 850.0

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec3 pos = vec3( fragCoord/iResolution.x - vec2( 0.5, 0.5 ), 0.0 );

    vec3 origin = vec3( 0.0, 0.0, 1.0 );
    vec3 ray_dir = pos - origin ;
    vec3 c = vec3( -1.05241, -.8134335, -.95455);

    vec3 col;
    float pitch = iTime / 5.0;
    float cos_a = cos(pitch);
    float sin_a = sin(pitch);
    float yaw = 0.6+sin(iTime/2.0)/3.0;
    float sin_b = sin( yaw );
    float cos_b = cos( yaw );
    mat3 rot_m = mat3( cos_a, 0.0, -sin_a,
                    sin_a*-sin_b, cos_b, cos_a*-sin_b,
                    sin_a*cos_b, sin_b, cos_a*cos_b );                        

    origin = rot_m*vec3( 0,0,1.0  + sin(iTime*0.9 ) * .55 ) ;
    origin.y = .9;
    ray_dir = rot_m*ray_dir;

    for( float depth = 60.0; depth < steps; depth ++ ) {
        pos = origin +  depth * ray_dir * 0.003;
        ray_dir = ray_dir * 1.003;
        vec3 prev  = vec3( 1,1,1), prev_prev;
        for( int idx=0; idx< 7;idx++ ) 
        {
            prev_prev = prev;
            prev = pos;
            pos = abs(pos);
            pos =  pos / dot( pos, pos ) + c;
            if( dot( pos,pos  )  > 6.89 ) 
            {
                if( idx  >3  ) 
                {
                    col = normalize( prev - prev_prev ) / 2.0 + vec3( 0.5, 0.5, 0.5 );
                } else  
                {

                  col = normalize( -prev-prev_prev ) / 2.0 + vec3( 0.5, 0.5, 0.5 );
                } 
                depth = 100000.0;
                break;
            }
        }
    }    
    fragColor = vec4(col,1.0);
}