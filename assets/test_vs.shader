DXBCцrkWLZpLуWB®ря   l@     <   (  h  p=  м=  А?  @  Aon9д  д   ю€§  @    $   <   <   $  <                        ю€ю€÷ DBUG(             T      X      –    D:\Profiles\cs2083\tofu\tofu\test_vs.hlsl ЂЂ(     €€`    €€l      x      И      Ш      ®  !   Є  !   »  !   Ў  !   и  "   ш  "     "     "   (  %   8  &   D  (   P  (   \  (   h     t     Д     Ф  main input position            normal Ђ           tangent texcoord ЂЂЂ               ,  4  D  4  L  X         h              
  €€€€    L  X         ∞        €€€€   €€€€       €€€€      €€€€   €€€€  output Ђ       ∞               €€€€pos      €€€€€€   €€ €€€€   €€€€ €€   €€€€€€      €€€€€€   €€ €€€€   €€€€ €€	   €€€€€€ 
     €€€€€€   €€ €€€€   €€€€ €€   €€€€€€     И     Ш        ј     –             $      <       @  Microsoft (R) HLSL Shader Compiler 6.3.9600.16384 ЂЂ    А  Р   А Р	    А  дР д†	    А  дР д†	    А  дР д†	    А  дР д†	   А  дА д†	   А  дА д†	   А  дА д†	   А  дА д†	    А дА	 д†	    А дА
 д†	    А дА д†	    А дА д†    А  дА   А дР    А  дА    А  дА    а дА   А  €А  д†    ј  дА дА    ј  дА€€  SHDR8  @  О   Y  FО         Y  FО        _  т     _  2    g  т         e  2     h             F     FО            "      F     FО           B      F     FО           В      F     FО                F     FО           "     F     FО          B     F     FО          В     F     FО                F    FО          "      F    FО          B      F    FО          В      F    FО        6  т      F     6  2     F    6  т      F     6  2     F     >  SPDB 6  Microsoft C/C++ MSF 7.00
DS            Ф                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ј€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€8  ш€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€       <       €€€€                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         Ф.1л7Z    H‘olLњђ9МЮЧ@                          AС2                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ;

VS_OUTPUT main(VS_INPUT input)
{
	//float4 pos = mul(mul(mul(input.position, matModel), matView), matProj);
	//float4 pos = mul(mul(input.position, matView), matProj);

	float4 pos = mul(input.position, matModel);
	pos = mul(pos, matView);
	pos = mul(pos, matProj);

	VS_OUTPUT output;
	output.position = pos;
	output.texcoord = input.texcoord;

	return output;
}
    V   +               ,                                                                                                    ws  3M  /,  “m  IL  L  √I  ‘{  р1  ‘(  №|  ≠Z  б/                                                                                                                                                                                                                                                                                                                                                                                                                                                                             cbuffer InstanceConstants : register (b0)
{
	matrix		matModel;
}

cbuffer FrameConstants : register (b1)
{
	matrix		matView;
	matrix		matProj;
	float4		cameraPos;
};

struct VS_INPUT
{
	float4		position	: POSITION;
	float3		normal		: NORMAL;
	float3		tangent		: TANGENT;
	float2		texcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4		position	: SV_POSITION;
	float2		texcoord	: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	//float4 pos = mul(mul(mul(input.position, matModel), matView), matProj);
	//float4 pos = mul(mul(input.position, matView), matProj);

	float4 pos = mul(input.position, matModel);
	pos = mul(pos, matView);
	pos = mul(pos, matProj);

	VS_OUTPUT output;
	output.position = pos;
	output.texcoord = input.texcoord;

	return output;
}
                                                                                                                                                                                                                                  юпюп   u   D:\Profiles\cs2083\tofu\tofu\test_vs.hlsl  d:\profiles\cs2083\tofu\tofu\test_vs.hlsl cbuffer InstanceConstants : register (b0)
{
	matrix		matModel;
}

cbuffer FrameConstants : register (b1)
{
	matrix		matView;
	matrix		matProj;
	float4		cameraPos;
};

struct VS_INPUT
{
	float4		position	: POSITION;
	float3		normal		: NORMAL;
	float3		tangent		: TANGENT;
	float2		texcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4		position	: SV_POSITION;
	float2		texcoord	: TEXCOORD0;
}в0А   N н≤X”                                                               ,   (   в0ѕ%%9     +   ,                                                                                                                                                                                                                                                                                                                                                                                                                  J <      @А%   @А%Microsoft (R) HLSL Shader Compiler 6.3.9600.16384 > =hlslFlags 0x5 hlslTarget vs_4_0_level_9_3 hlslEntry main   *     Ф      ‘      ‘  	  d    †main . >  	 input                                  P     d    ‘     P    d    ‘    P    d    ‘    P    d    ‘    P    d    ‘    P    d    ‘    P    d    ‘    P    d    ‘     P     d    ‘$    P  $  d    ‘(    P  (  d    ‘0    P  ,  d    ‘4   : >  И <main return value>                                P    d    ‘    P    d    ‘    P     d    ‘     P    d    ‘    P    d    ‘    P    d    ‘   * >    pos                                P      Д    іА А      P     §    ФА А     P     ƒ    tА А     P     д    TА А     P         А     P     $   А     P     D   А     P     d   А    . >   output                                 P      ш   @      P     ш   @     P     ш   @     P     ш   @     P        ,     P        ,      ф         эЌ°ЄшдіJD=ш`	Ђ  т   ∞        8      "   §  d      Аd       Д      АД       §      А§       ƒ      Аƒ       д   !  Ад   !     !  А  !   $  !  А$  !   D  !  АD  !   d  "  Аd  "   Д  "  АД  "   §  "  А§  "   ƒ  "  Аƒ  "   д  %  Ад  %   ш  &  Аш  &     (  А  (      (  А   (   4  (  А4  (    ,  +  ,  +  ,  +  ,  +                                      "  !             ц                    4   P                                                                                                                                                                                                                                                                                                                                                                                    18        h  
 €€   А      4   4      <        @       float4 утс @       float3 утс @       float2 утсR       position с    normal утс    tangent тс   ( texcoord с              0 VS_INPUT с
      *       position с    texcoord с               VS_OUTPUT 
             @             @ float4x4 
 
    
                                                                                                      18              €€   А                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  D3DSHDR 8                             `                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        €€€€	/с      Q                  5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  $                                                                                                                                                                                                                                                                                                                                                                                                                                                                %    Р    main   Q       €€€€€€matModel     Q      €€€€€€matView  Q    @ €€€€€€matProj                                                                                                                                                                                                                                                                                                                                                                                                                                                €€€€	/с                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            €€€€w	1     Л Ј√   L       ,   8                                    8     `             	 Ш      д     x™М         main none   -Ї.с       8     `                    €€€€    8        €€€€    €€€€         D:\Profiles\cs2083\tofu\tofu\test_vs.hlsl   юпюп                  €€€€€€€€€€ €€€€€€€€€€                                                                                                                                                                                                 Ф.1л7Z    H‘olLњђ9МЮЧ@W   /LinkInfo /names /src/headerblock /src/files/d:\profiles\cs2083\tofu\tofu\test_vs.hlsl                 "            
                 AС2                                                                                                                                                                                                                                                                                                                                                    ≥   †  ?  8       •  А     Р  <       (   D  ,   l                  
            	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            STATt                                                                                                                 RDEFМ     А          ю€  X  \                             n                            InstanceConstants FrameConstants ЂЂЂ\      ∞   @           n      д   Р           »       @      ‘       matModel ЂЂЂ            ,      @      ‘       4  @   @      ‘       <  А          H      matView matProj cameraPos ЂЂ            Microsoft (R) HLSL Shader Compiler 6.3.9600.16384 ЂЂISGNМ         h                    q                    x                    А                   POSITION NORMAL TANGENT TEXCOORD ЂЂЂOSGNP         8                    D                   SV_POSITION TEXCOORD ЂЂЂ