#line 1 "C:\\Users\\Senpai\\source\\repos\\PCG_and_graphics_stale_memes\\Engine\\ShGen\\genVS.hlsl"
cbuffer PerCameraBuffer : register ( b10 ) 
{ 
    matrix projectionMatrix ; 
} ; 

cbuffer PerFrameBuffer : register ( b11 ) 
{ 
    matrix viewMatrix ; 
    float dTime ; 
    float eTime ; 
    float2 padding ; 
} ; 

cbuffer WMBuffer : register ( b0 ) 
{ 
    matrix worldMatrix ; 
} ; 

#line 20
struct VertexInputType 
{ 
    float4 position : POSITION ; 
    
#line 26
    
    
#line 30
    
    
#line 34
    
    
#line 38
    
} ; 

struct PixelInputType 
{ 
    float4 position : SV_POSITION ; 
    
#line 47
    
    
#line 51
    
    
#line 55
    
    
#line 59
    
} ; 

#line 63
PixelInputType main ( VertexInputType input ) 
{ 
    PixelInputType output ; 
    
#line 70
    
    output . position = mul ( input . position , worldMatrix ) ; 
    output . position = mul ( output . position , viewMatrix ) ; 
    
    
    output . position = mul ( output . position , projectionMatrix ) ; 
    
#line 79
    
    
#line 84
    
    
#line 88
    
    
    return output ; 
}  