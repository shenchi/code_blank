DXBCHСњ"ГеOёћЌџRO   и>     <   P  <  D<  ј<   >  і>  Aon9     ю€ћ  @    $   <   <   $  <                        ю€ю€ѓ DBUG(   Д         X      \      H  д   C:\Users\SC-rMBP\workspace\tofu\test_vs.hlsl ЂЂЂ(     €€ƒ     –     а     р                      0     @     P     `     p     А     Р     Ь     ђ     Љ  main input position            normal Ђ           tangent texcoord ЂЂЂ           п   ш            (  4         D                                    €€€€   €€€€  pos      €€€€€€   €€ €€€€   €€€€ €€   €€€€€€      €€€€€€   €€ €€€€   €€€€ €€   €€€€€€ 	     €€€€€€
   €€ €€€€   €€€€ €€   €€€€€€ д   й   d     t      д   А     Р      і  ш      Є  Microsoft (R) HLSL Shader Compiler 6.3.9600.16384 ЂЂ    А  Р	    А  дР д†	    А  дР д†	    А  дР д†	    А  дР д†	   А  дА д†	   А  дА д†	   А  дА д†	   А  дА д†	    А дА	 д†	    А дА
 д†	    А дА д†	    А дА д†    А  дА   А  €А  д†    ј  дА дА    ј  дА€€  SHDRд  @  y   Y  FО         Y  FО        _  т     g  т         h             F     FО            "      F     FО           B      F     FО           В      F     FО                F     FО           "     F     FО          B     F     FО          В     F     FО                F    FО          "      F    FО          B      F    FО          В      F    FО        6  т      F     >  SPDB 6  Microsoft C/C++ MSF 7.00
DS            Р                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ј€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€8  ь€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€       <       €€€€                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         Ф.1и№Z   J≥Uyѕ€™Iѓ{:4!4н                          AС2                                                                                                                                                                                                                                                                                                                                                                                                                                                                    float4 pos = mul(mul(input.position, matView), matProj);

	float4 pos = mul(input.position, matModel);
	pos = mul(pos, matView);
	pos = mul(pos, matProj);

	return pos;
}
        .   \          /                                                                                                                                                                                                                                                                                                                          ws  3M  /,  “m  IL  L  Кn  ъr  №|  n  У*                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     cbuffer InstanceConstants : register (b0)
{
	matrix		matModel;
}

cbuffer FrameConstants : register (b1)
{
	matrix		matView;
	matrix		matProj;
};

struct VS_INPUT
{
	float4		position	: POSITION;
	float3		normal		: NORMAL;
	float3		tangent		: TANGENT;
	float2		texcoord	: TEXCOORD0;
};

float4 main(VS_INPUT input) : SV_POSITION
{
	//float4 pos = mul(mul(mul(input.position, matModel), matView), matProj);
	//float4 pos = mul(mul(input.position, matView), matProj);

	float4 pos = mul(input.position, matModel);
	pos = mul(pos, matView);
	pos = mul(pos, matProj);

	return pos;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                             юпюп   †   C:\Users\SC-rMBP\workspace\tofu\test_vs.hlsl  c:\users\sc-rmbp\workspace\tofu\test_vs.hlsl cbuffer InstanceConstants : register (b0)
{
	matrix		matModel;
}

cbuffer FrameConstants : register (b1)
{
	matrix		matView;
	matrix		matProj;
};

struct VS_INPUT
{
	float4		position	: POSITION;
	float3		normal		: NORMAL;
	float3		tangent		: TANGENT;
	float2		texcoord	: TEXCOORD0;
};

float4 main(VS_INPUT input) : SV_POSITION
{
	//float4 pos = mul(mul(mul(input.position, matModel), matView), matProj);
	//в0А   LЌдW”                                                               /   (   в0НпЬ|C     .   /                                                                                                                                                                                                                                                                                                                                                                                                                  J <      @А%   @А%Microsoft (R) HLSL Shader Compiler 6.3.9600.16384 > =hlslFlags 0x5 hlslTarget vs_4_0_level_9_3 hlslEntry main   *     §      Ш      Ш    L    †main . >  	 input                                  P     L    Ш     P    L    Ш    P    L    Ш    P    L    Ш    P    L    Ш    P    L    Ш    P    L    Ш    P    L    Ш     P     L    Ш$    P  $  L    Ш(    P  (  L    Ш0    P  ,  L    Ш4   : >  И <main return value>                                P     L    Ш     P    L    Ш    P    L    Ш    P    L    Ш   * >    pos                                P      l    xА А      P     М    XА А     P     ђ    8А А     P     ћ    А А     P      м    А     P        А     P     ,   А     P     L   А      ф         А8…јdбh@abЮ5
  т   h        д         \  L     АL      l     Аl      М     АМ      ђ     Ађ      ћ     Аћ      м     Ам          А     ,    А,     L    АL     l    Аl     М    АМ     ђ    Ађ     ћ    Аћ     а    Аа      ,  +  ,  +  ,  +  ,  +                                         ц                    4   P                                                                                                                                                                            18          
 €€   А      ,   ,      4        @       float4 утс @       float3 утс @       float2 утсR       position с    normal утс    tangent тс   ( texcoord с              0 VS_INPUT с
      
              @             @ float4x4 
     
 	                                                                                                                                                                                 18              €€   А                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  D3DSHDR д                             `                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        €€€€	/с      Q                  5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  $                                                                                                                                                                                                                                                                                                                                                                                                                                                                %    Р    main   Q
       €€€€€€matModel     Q
      €€€€€€matView  Q
    @ €€€€€€matProj                                                                                                                                                                                                                                                                                                                                                                                                                                                €€€€	/с                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            €€€€w	1     Л Ј√   L       ,   <                                    д     `             	 ®      Ь     h2\        main none   -Ї.с       д     `                    €€€€    д        €€€€    €€€€         C:\Users\SC-rMBP\workspace\tofu\test_vs.hlsl    юпюп                  €€€€€€€€€€ €€€€€€€€€€                                                                                                                                                                                             Ф.1и№Z   J≥Uyѕ€™Iѓ{:4!4нZ   /LinkInfo /names /src/headerblock /src/files/c:\users\sc-rmbp\workspace\tofu\test_vs.hlsl          :             
             "          AС2                                                                                                                                                                                                                                                                                                                                                 ґ   T  C  8       –  А   C  X  4       (   D  ,   l                  
            	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             STATt                                                                                                                 RDEFX     А          ю€  $  \                             n                            InstanceConstants FrameConstants ЂЂЂ\      ∞   @           n      д   А           »       @      ‘       matModel ЂЂЂ                  @      ‘         @   @      ‘       matView matProj Microsoft (R) HLSL Shader Compiler 6.3.9600.16384 ЂЂISGNМ         h                    q                    x                    А                    POSITION NORMAL TANGENT TEXCOORD ЂЂЂOSGN,                              SV_POSITION 