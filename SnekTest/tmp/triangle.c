// triangle.src
#include <snek.h>


struct Vertex;
struct vec2{
	float x;
	float y;
};
struct vec3{
	float x;
	float y;
	float z;
};
struct vec4{
	float x;
	float y;
	float z;
	float w;
};
struct Vertex{
	struct vec3 position;
	struct vec4 color;
};
typedef struct{struct Vertex data[3];}A3SVertex;
struct SDL_GPUShader;
enum SDL_GPUShaderStage:i32{
	SDL_GPU_SHADERSTAGE_VERTEX,
	SDL_GPU_SHADERSTAGE_FRAGMENT,
};
struct SDL_GPUDevice;
typedef u8 Uint8;
typedef u32 Uint32;
typedef Uint32 SDL_GPUShaderFormat;
typedef Uint32 SDL_PropertiesID;
struct SDL_GPUShaderCreateInfo{
	u64 code_size;
	Uint8* code;
	i8* entrypoint;
	SDL_GPUShaderFormat format;
	enum SDL_GPUShaderStage stage;
	Uint32 num_samplers;
	Uint32 num_storage_textures;
	Uint32 num_storage_buffers;
	Uint32 num_uniform_buffers;
	SDL_PropertiesID props;
};
enum SDL_GPUBlendFactor:i32{
	SDL_GPU_BLENDFACTOR_INVALID,
	SDL_GPU_BLENDFACTOR_ZERO,
	SDL_GPU_BLENDFACTOR_ONE,
	SDL_GPU_BLENDFACTOR_SRC_COLOR,
	SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
	SDL_GPU_BLENDFACTOR_DST_COLOR,
	SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR,
	SDL_GPU_BLENDFACTOR_SRC_ALPHA,
	SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
	SDL_GPU_BLENDFACTOR_DST_ALPHA,
	SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
	SDL_GPU_BLENDFACTOR_CONSTANT_COLOR,
	SDL_GPU_BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR,
	SDL_GPU_BLENDFACTOR_SRC_ALPHA_SATURATE,
};
enum SDL_GPUBlendOp:i32{
	SDL_GPU_BLENDOP_INVALID,
	SDL_GPU_BLENDOP_ADD,
	SDL_GPU_BLENDOP_SUBTRACT,
	SDL_GPU_BLENDOP_REVERSE_SUBTRACT,
	SDL_GPU_BLENDOP_MIN,
	SDL_GPU_BLENDOP_MAX,
};
typedef Uint8 SDL_GPUColorComponentFlags;
struct SDL_GPUColorTargetBlendState{
	enum SDL_GPUBlendFactor src_color_blendfactor;
	enum SDL_GPUBlendFactor dst_color_blendfactor;
	enum SDL_GPUBlendOp color_blend_op;
	enum SDL_GPUBlendFactor src_alpha_blendfactor;
	enum SDL_GPUBlendFactor dst_alpha_blendfactor;
	enum SDL_GPUBlendOp alpha_blend_op;
	SDL_GPUColorComponentFlags color_write_mask;
	bool enable_blend;
	bool enable_color_write_mask;
	Uint8 padding1;
	Uint8 padding2;
};
typedef Uint32 SDL_InitFlags;
struct SDL_Window;
typedef u64 Uint64;
typedef Uint64 SDL_WindowFlags;
typedef Uint32 SDL_GPUBufferUsageFlags;
struct SDL_GPUBufferCreateInfo{
	SDL_GPUBufferUsageFlags usage;
	Uint32 size;
	SDL_PropertiesID props;
};
struct SDL_GPUBuffer;
enum SDL_GPUTransferBufferUsage:i32{
	SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
	SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD,
};
struct SDL_GPUTransferBufferCreateInfo{
	enum SDL_GPUTransferBufferUsage usage;
	Uint32 size;
	SDL_PropertiesID props;
};
struct SDL_GPUTransferBuffer;
struct SDL_GPUCommandBuffer;
struct SDL_GPUCopyPass;
struct SDL_GPUTransferBufferLocation{
	struct SDL_GPUTransferBuffer* transfer_buffer;
	Uint32 offset;
};
struct SDL_GPUBufferRegion{
	struct SDL_GPUBuffer* buffer;
	Uint32 offset;
	Uint32 size;
};
enum SDL_GPUVertexInputRate:i32{
	SDL_GPU_VERTEXINPUTRATE_VERTEX,
	SDL_GPU_VERTEXINPUTRATE_INSTANCE,
};
struct SDL_GPUVertexBufferDescription{
	Uint32 slot;
	Uint32 pitch;
	enum SDL_GPUVertexInputRate input_rate;
	Uint32 instance_step_rate;
};
typedef struct{struct SDL_GPUVertexBufferDescription data[1];}A1SSDL_GPUVertexBufferDescription;
enum SDL_GPUVertexElementFormat:i32{
	SDL_GPU_VERTEXELEMENTFORMAT_INVALID,
	SDL_GPU_VERTEXELEMENTFORMAT_INT,
	SDL_GPU_VERTEXELEMENTFORMAT_INT2,
	SDL_GPU_VERTEXELEMENTFORMAT_INT3,
	SDL_GPU_VERTEXELEMENTFORMAT_INT4,
	SDL_GPU_VERTEXELEMENTFORMAT_UINT,
	SDL_GPU_VERTEXELEMENTFORMAT_UINT2,
	SDL_GPU_VERTEXELEMENTFORMAT_UINT3,
	SDL_GPU_VERTEXELEMENTFORMAT_UINT4,
	SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
	SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
	SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
	SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
	SDL_GPU_VERTEXELEMENTFORMAT_BYTE2,
	SDL_GPU_VERTEXELEMENTFORMAT_BYTE4,
	SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2,
	SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4,
	SDL_GPU_VERTEXELEMENTFORMAT_BYTE2_NORM,
	SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM,
	SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2_NORM,
	SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
	SDL_GPU_VERTEXELEMENTFORMAT_SHORT2,
	SDL_GPU_VERTEXELEMENTFORMAT_SHORT4,
	SDL_GPU_VERTEXELEMENTFORMAT_USHORT2,
	SDL_GPU_VERTEXELEMENTFORMAT_USHORT4,
	SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM,
	SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM,
	SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM,
	SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM,
	SDL_GPU_VERTEXELEMENTFORMAT_HALF2,
	SDL_GPU_VERTEXELEMENTFORMAT_HALF4,
};
struct SDL_GPUVertexAttribute{
	Uint32 location;
	Uint32 buffer_slot;
	enum SDL_GPUVertexElementFormat format;
	Uint32 offset;
};
typedef struct{struct SDL_GPUVertexAttribute data[2];}A2SSDL_GPUVertexAttribute;
enum SDL_GPUTextureFormat:i32{
	SDL_GPU_TEXTUREFORMAT_INVALID,
	SDL_GPU_TEXTUREFORMAT_A8_UNORM,
	SDL_GPU_TEXTUREFORMAT_R8_UNORM,
	SDL_GPU_TEXTUREFORMAT_R8G8_UNORM,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
	SDL_GPU_TEXTUREFORMAT_R16_UNORM,
	SDL_GPU_TEXTUREFORMAT_R16G16_UNORM,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UNORM,
	SDL_GPU_TEXTUREFORMAT_R10G10B10A2_UNORM,
	SDL_GPU_TEXTUREFORMAT_B5G6R5_UNORM,
	SDL_GPU_TEXTUREFORMAT_B5G5R5A1_UNORM,
	SDL_GPU_TEXTUREFORMAT_B4G4R4A4_UNORM,
	SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM,
	SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM,
	SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM,
	SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM,
	SDL_GPU_TEXTUREFORMAT_BC4_R_UNORM,
	SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM,
	SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM,
	SDL_GPU_TEXTUREFORMAT_BC6H_RGB_FLOAT,
	SDL_GPU_TEXTUREFORMAT_BC6H_RGB_UFLOAT,
	SDL_GPU_TEXTUREFORMAT_R8_SNORM,
	SDL_GPU_TEXTUREFORMAT_R8G8_SNORM,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_SNORM,
	SDL_GPU_TEXTUREFORMAT_R16_SNORM,
	SDL_GPU_TEXTUREFORMAT_R16G16_SNORM,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_SNORM,
	SDL_GPU_TEXTUREFORMAT_R16_FLOAT,
	SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
	SDL_GPU_TEXTUREFORMAT_R32_FLOAT,
	SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT,
	SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
	SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT,
	SDL_GPU_TEXTUREFORMAT_R8_UINT,
	SDL_GPU_TEXTUREFORMAT_R8G8_UINT,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UINT,
	SDL_GPU_TEXTUREFORMAT_R16_UINT,
	SDL_GPU_TEXTUREFORMAT_R16G16_UINT,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UINT,
	SDL_GPU_TEXTUREFORMAT_R32_UINT,
	SDL_GPU_TEXTUREFORMAT_R32G32_UINT,
	SDL_GPU_TEXTUREFORMAT_R32G32B32A32_UINT,
	SDL_GPU_TEXTUREFORMAT_R8_INT,
	SDL_GPU_TEXTUREFORMAT_R8G8_INT,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_INT,
	SDL_GPU_TEXTUREFORMAT_R16_INT,
	SDL_GPU_TEXTUREFORMAT_R16G16_INT,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_INT,
	SDL_GPU_TEXTUREFORMAT_R32_INT,
	SDL_GPU_TEXTUREFORMAT_R32G32_INT,
	SDL_GPU_TEXTUREFORMAT_R32G32B32A32_INT,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_D16_UNORM,
	SDL_GPU_TEXTUREFORMAT_D24_UNORM,
	SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
	SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
	SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
	SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x4_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x5_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x5_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x6_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x5_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x6_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x8_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x10_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x10_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x12_UNORM,
	SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x4_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x5_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x5_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x6_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x5_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x6_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x8_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x10_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x10_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x12_UNORM_SRGB,
	SDL_GPU_TEXTUREFORMAT_ASTC_4x4_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x4_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x5_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x5_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x6_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x5_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x6_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x8_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x5_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x6_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x8_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x10_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x10_FLOAT,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x12_FLOAT,
};
struct SDL_GPUColorTargetDescription{
	enum SDL_GPUTextureFormat format;
	struct SDL_GPUColorTargetBlendState blend_state;
};
typedef struct{struct SDL_GPUColorTargetDescription data[1];}A1SSDL_GPUColorTargetDescription;
struct SDL_GPUVertexInputState{
	struct SDL_GPUVertexBufferDescription* vertex_buffer_descriptions;
	Uint32 num_vertex_buffers;
	struct SDL_GPUVertexAttribute* vertex_attributes;
	Uint32 num_vertex_attributes;
};
enum SDL_GPUPrimitiveType:i32{
	SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
	SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP,
	SDL_GPU_PRIMITIVETYPE_LINELIST,
	SDL_GPU_PRIMITIVETYPE_LINESTRIP,
	SDL_GPU_PRIMITIVETYPE_POINTLIST,
};
enum SDL_GPUFillMode:i32{
	SDL_GPU_FILLMODE_FILL,
	SDL_GPU_FILLMODE_LINE,
};
enum SDL_GPUCullMode:i32{
	SDL_GPU_CULLMODE_NONE,
	SDL_GPU_CULLMODE_FRONT,
	SDL_GPU_CULLMODE_BACK,
};
enum SDL_GPUFrontFace:i32{
	SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
	SDL_GPU_FRONTFACE_CLOCKWISE,
};
struct SDL_GPURasterizerState{
	enum SDL_GPUFillMode fill_mode;
	enum SDL_GPUCullMode cull_mode;
	enum SDL_GPUFrontFace front_face;
	float depth_bias_constant_factor;
	float depth_bias_clamp;
	float depth_bias_slope_factor;
	bool enable_depth_bias;
	bool enable_depth_clip;
	Uint8 padding1;
	Uint8 padding2;
};
enum SDL_GPUSampleCount:i32{
	SDL_GPU_SAMPLECOUNT_1,
	SDL_GPU_SAMPLECOUNT_2,
	SDL_GPU_SAMPLECOUNT_4,
	SDL_GPU_SAMPLECOUNT_8,
};
struct SDL_GPUMultisampleState{
	enum SDL_GPUSampleCount sample_count;
	Uint32 sample_mask;
	bool enable_mask;
	bool enable_alpha_to_coverage;
	Uint8 padding2;
	Uint8 padding3;
};
enum SDL_GPUCompareOp:i32{
	SDL_GPU_COMPAREOP_INVALID,
	SDL_GPU_COMPAREOP_NEVER,
	SDL_GPU_COMPAREOP_LESS,
	SDL_GPU_COMPAREOP_EQUAL,
	SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
	SDL_GPU_COMPAREOP_GREATER,
	SDL_GPU_COMPAREOP_NOT_EQUAL,
	SDL_GPU_COMPAREOP_GREATER_OR_EQUAL,
	SDL_GPU_COMPAREOP_ALWAYS,
};
enum SDL_GPUStencilOp:i32{
	SDL_GPU_STENCILOP_INVALID,
	SDL_GPU_STENCILOP_KEEP,
	SDL_GPU_STENCILOP_ZERO,
	SDL_GPU_STENCILOP_REPLACE,
	SDL_GPU_STENCILOP_INCREMENT_AND_CLAMP,
	SDL_GPU_STENCILOP_DECREMENT_AND_CLAMP,
	SDL_GPU_STENCILOP_INVERT,
	SDL_GPU_STENCILOP_INCREMENT_AND_WRAP,
	SDL_GPU_STENCILOP_DECREMENT_AND_WRAP,
};
struct SDL_GPUStencilOpState{
	enum SDL_GPUStencilOp fail_op;
	enum SDL_GPUStencilOp pass_op;
	enum SDL_GPUStencilOp depth_fail_op;
	enum SDL_GPUCompareOp compare_op;
};
struct SDL_GPUDepthStencilState{
	enum SDL_GPUCompareOp compare_op;
	struct SDL_GPUStencilOpState back_stencil_state;
	struct SDL_GPUStencilOpState front_stencil_state;
	Uint8 compare_mask;
	Uint8 write_mask;
	bool enable_depth_test;
	bool enable_depth_write;
	bool enable_stencil_test;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
};
struct SDL_GPUGraphicsPipelineTargetInfo{
	struct SDL_GPUColorTargetDescription* color_target_descriptions;
	Uint32 num_color_targets;
	enum SDL_GPUTextureFormat depth_stencil_format;
	bool has_depth_stencil_target;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
};
struct SDL_GPUGraphicsPipelineCreateInfo{
	struct SDL_GPUShader* vertex_shader;
	struct SDL_GPUShader* fragment_shader;
	struct SDL_GPUVertexInputState vertex_input_state;
	enum SDL_GPUPrimitiveType primitive_type;
	struct SDL_GPURasterizerState rasterizer_state;
	struct SDL_GPUMultisampleState multisample_state;
	struct SDL_GPUDepthStencilState depth_stencil_state;
	struct SDL_GPUGraphicsPipelineTargetInfo target_info;
	SDL_PropertiesID props;
};
struct SDL_GPUGraphicsPipeline;
struct SDL_CommonEvent{
	Uint32 type;
	Uint32 reserved;
	Uint64 timestamp;
};
enum SDL_EventType:i32{
	SDL_EVENT_FIRST=0,
	SDL_EVENT_QUIT=256,
	SDL_EVENT_TERMINATING,
	SDL_EVENT_LOW_MEMORY,
	SDL_EVENT_WILL_ENTER_BACKGROUND,
	SDL_EVENT_DID_ENTER_BACKGROUND,
	SDL_EVENT_WILL_ENTER_FOREGROUND,
	SDL_EVENT_DID_ENTER_FOREGROUND,
	SDL_EVENT_LOCALE_CHANGED,
	SDL_EVENT_SYSTEM_THEME_CHANGED,
	SDL_EVENT_DISPLAY_ORIENTATION=337,
	SDL_EVENT_DISPLAY_ADDED,
	SDL_EVENT_DISPLAY_REMOVED,
	SDL_EVENT_DISPLAY_MOVED,
	SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED,
	SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED,
	SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED,
	SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED,
	SDL_EVENT_DISPLAY_FIRST,
	SDL_EVENT_DISPLAY_LAST,
	SDL_EVENT_WINDOW_SHOWN=514,
	SDL_EVENT_WINDOW_HIDDEN,
	SDL_EVENT_WINDOW_EXPOSED,
	SDL_EVENT_WINDOW_MOVED,
	SDL_EVENT_WINDOW_RESIZED,
	SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,
	SDL_EVENT_WINDOW_METAL_VIEW_RESIZED,
	SDL_EVENT_WINDOW_MINIMIZED,
	SDL_EVENT_WINDOW_MAXIMIZED,
	SDL_EVENT_WINDOW_RESTORED,
	SDL_EVENT_WINDOW_MOUSE_ENTER,
	SDL_EVENT_WINDOW_MOUSE_LEAVE,
	SDL_EVENT_WINDOW_FOCUS_GAINED,
	SDL_EVENT_WINDOW_FOCUS_LOST,
	SDL_EVENT_WINDOW_CLOSE_REQUESTED,
	SDL_EVENT_WINDOW_HIT_TEST,
	SDL_EVENT_WINDOW_ICCPROF_CHANGED,
	SDL_EVENT_WINDOW_DISPLAY_CHANGED,
	SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED,
	SDL_EVENT_WINDOW_SAFE_AREA_CHANGED,
	SDL_EVENT_WINDOW_OCCLUDED,
	SDL_EVENT_WINDOW_ENTER_FULLSCREEN,
	SDL_EVENT_WINDOW_LEAVE_FULLSCREEN,
	SDL_EVENT_WINDOW_DESTROYED,
	SDL_EVENT_WINDOW_HDR_STATE_CHANGED,
	SDL_EVENT_WINDOW_SETTINGS_CHANGED,
	SDL_EVENT_WINDOW_FIRST,
	SDL_EVENT_WINDOW_LAST,
	SDL_EVENT_KEY_DOWN=768,
	SDL_EVENT_KEY_UP,
	SDL_EVENT_TEXT_EDITING,
	SDL_EVENT_TEXT_INPUT,
	SDL_EVENT_KEYMAP_CHANGED,
	SDL_EVENT_KEYBOARD_ADDED,
	SDL_EVENT_KEYBOARD_REMOVED,
	SDL_EVENT_TEXT_EDITING_CANDIDATES,
	SDL_EVENT_SCREEN_KEYBOARD_SHOWN,
	SDL_EVENT_SCREEN_KEYBOARD_HIDDEN,
	SDL_EVENT_MOUSE_MOTION=1024,
	SDL_EVENT_MOUSE_BUTTON_DOWN,
	SDL_EVENT_MOUSE_BUTTON_UP,
	SDL_EVENT_MOUSE_WHEEL,
	SDL_EVENT_MOUSE_ADDED,
	SDL_EVENT_MOUSE_REMOVED,
	SDL_EVENT_JOYSTICK_AXIS_MOTION=1536,
	SDL_EVENT_JOYSTICK_BALL_MOTION,
	SDL_EVENT_JOYSTICK_HAT_MOTION,
	SDL_EVENT_JOYSTICK_BUTTON_DOWN,
	SDL_EVENT_JOYSTICK_BUTTON_UP,
	SDL_EVENT_JOYSTICK_ADDED,
	SDL_EVENT_JOYSTICK_REMOVED,
	SDL_EVENT_JOYSTICK_BATTERY_UPDATED,
	SDL_EVENT_JOYSTICK_UPDATE_COMPLETE,
	SDL_EVENT_GAMEPAD_AXIS_MOTION=1616,
	SDL_EVENT_GAMEPAD_BUTTON_DOWN,
	SDL_EVENT_GAMEPAD_BUTTON_UP,
	SDL_EVENT_GAMEPAD_ADDED,
	SDL_EVENT_GAMEPAD_REMOVED,
	SDL_EVENT_GAMEPAD_REMAPPED,
	SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN,
	SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION,
	SDL_EVENT_GAMEPAD_TOUCHPAD_UP,
	SDL_EVENT_GAMEPAD_SENSOR_UPDATE,
	SDL_EVENT_GAMEPAD_UPDATE_COMPLETE,
	SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED,
	SDL_EVENT_GAMEPAD_CAPSENSE_TOUCH,
	SDL_EVENT_GAMEPAD_CAPSENSE_RELEASE,
	SDL_EVENT_FINGER_DOWN=1792,
	SDL_EVENT_FINGER_UP,
	SDL_EVENT_FINGER_MOTION,
	SDL_EVENT_FINGER_CANCELED,
	SDL_EVENT_PINCH_BEGIN=1808,
	SDL_EVENT_PINCH_UPDATE,
	SDL_EVENT_PINCH_END,
	SDL_EVENT_CLIPBOARD_UPDATE=2304,
	SDL_EVENT_DROP_FILE=4096,
	SDL_EVENT_DROP_TEXT,
	SDL_EVENT_DROP_BEGIN,
	SDL_EVENT_DROP_COMPLETE,
	SDL_EVENT_DROP_POSITION,
	SDL_EVENT_AUDIO_DEVICE_ADDED=4352,
	SDL_EVENT_AUDIO_DEVICE_REMOVED,
	SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED,
	SDL_EVENT_SENSOR_UPDATE=4608,
	SDL_EVENT_PEN_PROXIMITY_IN=4864,
	SDL_EVENT_PEN_PROXIMITY_OUT,
	SDL_EVENT_PEN_DOWN,
	SDL_EVENT_PEN_UP,
	SDL_EVENT_PEN_BUTTON_DOWN,
	SDL_EVENT_PEN_BUTTON_UP,
	SDL_EVENT_PEN_MOTION,
	SDL_EVENT_PEN_AXIS,
	SDL_EVENT_CAMERA_DEVICE_ADDED=5120,
	SDL_EVENT_CAMERA_DEVICE_REMOVED,
	SDL_EVENT_CAMERA_DEVICE_APPROVED,
	SDL_EVENT_CAMERA_DEVICE_DENIED,
	SDL_EVENT_RENDER_TARGETS_RESET=8192,
	SDL_EVENT_RENDER_DEVICE_RESET,
	SDL_EVENT_RENDER_DEVICE_LOST,
	SDL_EVENT_PRIVATE0=16384,
	SDL_EVENT_PRIVATE1,
	SDL_EVENT_PRIVATE2,
	SDL_EVENT_PRIVATE3,
	SDL_EVENT_POLL_SENTINEL=32512,
	SDL_EVENT_USER=32768,
	SDL_EVENT_LAST=65535,
	SDL_EVENT_ENUM_PADDING=2147483647,
};
typedef Uint32 SDL_DisplayID;
typedef i32 Sint32;
struct SDL_DisplayEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_DisplayID displayID;
	Sint32 data1;
	Sint32 data2;
};
typedef Uint32 SDL_WindowID;
struct SDL_WindowEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	Sint32 data1;
	Sint32 data2;
};
typedef Uint32 SDL_KeyboardID;
struct SDL_KeyboardDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_KeyboardID which;
};
enum SDL_Scancode:i32{
	SDL_SCANCODE_UNKNOWN=0,
	SDL_SCANCODE_A=4,
	SDL_SCANCODE_B=5,
	SDL_SCANCODE_C=6,
	SDL_SCANCODE_D=7,
	SDL_SCANCODE_E=8,
	SDL_SCANCODE_F=9,
	SDL_SCANCODE_G=10,
	SDL_SCANCODE_H=11,
	SDL_SCANCODE_I=12,
	SDL_SCANCODE_J=13,
	SDL_SCANCODE_K=14,
	SDL_SCANCODE_L=15,
	SDL_SCANCODE_M=16,
	SDL_SCANCODE_N=17,
	SDL_SCANCODE_O=18,
	SDL_SCANCODE_P=19,
	SDL_SCANCODE_Q=20,
	SDL_SCANCODE_R=21,
	SDL_SCANCODE_S=22,
	SDL_SCANCODE_T=23,
	SDL_SCANCODE_U=24,
	SDL_SCANCODE_V=25,
	SDL_SCANCODE_W=26,
	SDL_SCANCODE_X=27,
	SDL_SCANCODE_Y=28,
	SDL_SCANCODE_Z=29,
	SDL_SCANCODE_1=30,
	SDL_SCANCODE_2=31,
	SDL_SCANCODE_3=32,
	SDL_SCANCODE_4=33,
	SDL_SCANCODE_5=34,
	SDL_SCANCODE_6=35,
	SDL_SCANCODE_7=36,
	SDL_SCANCODE_8=37,
	SDL_SCANCODE_9=38,
	SDL_SCANCODE_0=39,
	SDL_SCANCODE_RETURN=40,
	SDL_SCANCODE_ESCAPE=41,
	SDL_SCANCODE_BACKSPACE=42,
	SDL_SCANCODE_TAB=43,
	SDL_SCANCODE_SPACE=44,
	SDL_SCANCODE_MINUS=45,
	SDL_SCANCODE_EQUALS=46,
	SDL_SCANCODE_LEFTBRACKET=47,
	SDL_SCANCODE_RIGHTBRACKET=48,
	SDL_SCANCODE_BACKSLASH=49,
	SDL_SCANCODE_NONUSHASH=50,
	SDL_SCANCODE_SEMICOLON=51,
	SDL_SCANCODE_APOSTROPHE=52,
	SDL_SCANCODE_GRAVE=53,
	SDL_SCANCODE_COMMA=54,
	SDL_SCANCODE_PERIOD=55,
	SDL_SCANCODE_SLASH=56,
	SDL_SCANCODE_CAPSLOCK=57,
	SDL_SCANCODE_F1=58,
	SDL_SCANCODE_F2=59,
	SDL_SCANCODE_F3=60,
	SDL_SCANCODE_F4=61,
	SDL_SCANCODE_F5=62,
	SDL_SCANCODE_F6=63,
	SDL_SCANCODE_F7=64,
	SDL_SCANCODE_F8=65,
	SDL_SCANCODE_F9=66,
	SDL_SCANCODE_F10=67,
	SDL_SCANCODE_F11=68,
	SDL_SCANCODE_F12=69,
	SDL_SCANCODE_PRINTSCREEN=70,
	SDL_SCANCODE_SCROLLLOCK=71,
	SDL_SCANCODE_PAUSE=72,
	SDL_SCANCODE_INSERT=73,
	SDL_SCANCODE_HOME=74,
	SDL_SCANCODE_PAGEUP=75,
	SDL_SCANCODE_DELETE=76,
	SDL_SCANCODE_END=77,
	SDL_SCANCODE_PAGEDOWN=78,
	SDL_SCANCODE_RIGHT=79,
	SDL_SCANCODE_LEFT=80,
	SDL_SCANCODE_DOWN=81,
	SDL_SCANCODE_UP=82,
	SDL_SCANCODE_NUMLOCKCLEAR=83,
	SDL_SCANCODE_KP_DIVIDE=84,
	SDL_SCANCODE_KP_MULTIPLY=85,
	SDL_SCANCODE_KP_MINUS=86,
	SDL_SCANCODE_KP_PLUS=87,
	SDL_SCANCODE_KP_ENTER=88,
	SDL_SCANCODE_KP_1=89,
	SDL_SCANCODE_KP_2=90,
	SDL_SCANCODE_KP_3=91,
	SDL_SCANCODE_KP_4=92,
	SDL_SCANCODE_KP_5=93,
	SDL_SCANCODE_KP_6=94,
	SDL_SCANCODE_KP_7=95,
	SDL_SCANCODE_KP_8=96,
	SDL_SCANCODE_KP_9=97,
	SDL_SCANCODE_KP_0=98,
	SDL_SCANCODE_KP_PERIOD=99,
	SDL_SCANCODE_NONUSBACKSLASH=100,
	SDL_SCANCODE_APPLICATION=101,
	SDL_SCANCODE_POWER=102,
	SDL_SCANCODE_KP_EQUALS=103,
	SDL_SCANCODE_F13=104,
	SDL_SCANCODE_F14=105,
	SDL_SCANCODE_F15=106,
	SDL_SCANCODE_F16=107,
	SDL_SCANCODE_F17=108,
	SDL_SCANCODE_F18=109,
	SDL_SCANCODE_F19=110,
	SDL_SCANCODE_F20=111,
	SDL_SCANCODE_F21=112,
	SDL_SCANCODE_F22=113,
	SDL_SCANCODE_F23=114,
	SDL_SCANCODE_F24=115,
	SDL_SCANCODE_EXECUTE=116,
	SDL_SCANCODE_HELP=117,
	SDL_SCANCODE_MENU=118,
	SDL_SCANCODE_SELECT=119,
	SDL_SCANCODE_STOP=120,
	SDL_SCANCODE_AGAIN=121,
	SDL_SCANCODE_UNDO=122,
	SDL_SCANCODE_CUT=123,
	SDL_SCANCODE_COPY=124,
	SDL_SCANCODE_PASTE=125,
	SDL_SCANCODE_FIND=126,
	SDL_SCANCODE_MUTE=127,
	SDL_SCANCODE_VOLUMEUP=128,
	SDL_SCANCODE_VOLUMEDOWN=129,
	SDL_SCANCODE_KP_COMMA=133,
	SDL_SCANCODE_KP_EQUALSAS400=134,
	SDL_SCANCODE_INTERNATIONAL1=135,
	SDL_SCANCODE_INTERNATIONAL2=136,
	SDL_SCANCODE_INTERNATIONAL3=137,
	SDL_SCANCODE_INTERNATIONAL4=138,
	SDL_SCANCODE_INTERNATIONAL5=139,
	SDL_SCANCODE_INTERNATIONAL6=140,
	SDL_SCANCODE_INTERNATIONAL7=141,
	SDL_SCANCODE_INTERNATIONAL8=142,
	SDL_SCANCODE_INTERNATIONAL9=143,
	SDL_SCANCODE_LANG1=144,
	SDL_SCANCODE_LANG2=145,
	SDL_SCANCODE_LANG3=146,
	SDL_SCANCODE_LANG4=147,
	SDL_SCANCODE_LANG5=148,
	SDL_SCANCODE_LANG6=149,
	SDL_SCANCODE_LANG7=150,
	SDL_SCANCODE_LANG8=151,
	SDL_SCANCODE_LANG9=152,
	SDL_SCANCODE_ALTERASE=153,
	SDL_SCANCODE_SYSREQ=154,
	SDL_SCANCODE_CANCEL=155,
	SDL_SCANCODE_CLEAR=156,
	SDL_SCANCODE_PRIOR=157,
	SDL_SCANCODE_RETURN2=158,
	SDL_SCANCODE_SEPARATOR=159,
	SDL_SCANCODE_OUT=160,
	SDL_SCANCODE_OPER=161,
	SDL_SCANCODE_CLEARAGAIN=162,
	SDL_SCANCODE_CRSEL=163,
	SDL_SCANCODE_EXSEL=164,
	SDL_SCANCODE_KP_00=176,
	SDL_SCANCODE_KP_000=177,
	SDL_SCANCODE_THOUSANDSSEPARATOR=178,
	SDL_SCANCODE_DECIMALSEPARATOR=179,
	SDL_SCANCODE_CURRENCYUNIT=180,
	SDL_SCANCODE_CURRENCYSUBUNIT=181,
	SDL_SCANCODE_KP_LEFTPAREN=182,
	SDL_SCANCODE_KP_RIGHTPAREN=183,
	SDL_SCANCODE_KP_LEFTBRACE=184,
	SDL_SCANCODE_KP_RIGHTBRACE=185,
	SDL_SCANCODE_KP_TAB=186,
	SDL_SCANCODE_KP_BACKSPACE=187,
	SDL_SCANCODE_KP_A=188,
	SDL_SCANCODE_KP_B=189,
	SDL_SCANCODE_KP_C=190,
	SDL_SCANCODE_KP_D=191,
	SDL_SCANCODE_KP_E=192,
	SDL_SCANCODE_KP_F=193,
	SDL_SCANCODE_KP_XOR=194,
	SDL_SCANCODE_KP_POWER=195,
	SDL_SCANCODE_KP_PERCENT=196,
	SDL_SCANCODE_KP_LESS=197,
	SDL_SCANCODE_KP_GREATER=198,
	SDL_SCANCODE_KP_AMPERSAND=199,
	SDL_SCANCODE_KP_DBLAMPERSAND=200,
	SDL_SCANCODE_KP_VERTICALBAR=201,
	SDL_SCANCODE_KP_DBLVERTICALBAR=202,
	SDL_SCANCODE_KP_COLON=203,
	SDL_SCANCODE_KP_HASH=204,
	SDL_SCANCODE_KP_SPACE=205,
	SDL_SCANCODE_KP_AT=206,
	SDL_SCANCODE_KP_EXCLAM=207,
	SDL_SCANCODE_KP_MEMSTORE=208,
	SDL_SCANCODE_KP_MEMRECALL=209,
	SDL_SCANCODE_KP_MEMCLEAR=210,
	SDL_SCANCODE_KP_MEMADD=211,
	SDL_SCANCODE_KP_MEMSUBTRACT=212,
	SDL_SCANCODE_KP_MEMMULTIPLY=213,
	SDL_SCANCODE_KP_MEMDIVIDE=214,
	SDL_SCANCODE_KP_PLUSMINUS=215,
	SDL_SCANCODE_KP_CLEAR=216,
	SDL_SCANCODE_KP_CLEARENTRY=217,
	SDL_SCANCODE_KP_BINARY=218,
	SDL_SCANCODE_KP_OCTAL=219,
	SDL_SCANCODE_KP_DECIMAL=220,
	SDL_SCANCODE_KP_HEXADECIMAL=221,
	SDL_SCANCODE_LCTRL=224,
	SDL_SCANCODE_LSHIFT=225,
	SDL_SCANCODE_LALT=226,
	SDL_SCANCODE_LGUI=227,
	SDL_SCANCODE_RCTRL=228,
	SDL_SCANCODE_RSHIFT=229,
	SDL_SCANCODE_RALT=230,
	SDL_SCANCODE_RGUI=231,
	SDL_SCANCODE_MODE=257,
	SDL_SCANCODE_SLEEP=258,
	SDL_SCANCODE_WAKE=259,
	SDL_SCANCODE_CHANNEL_INCREMENT=260,
	SDL_SCANCODE_CHANNEL_DECREMENT=261,
	SDL_SCANCODE_MEDIA_PLAY=262,
	SDL_SCANCODE_MEDIA_PAUSE=263,
	SDL_SCANCODE_MEDIA_RECORD=264,
	SDL_SCANCODE_MEDIA_FAST_FORWARD=265,
	SDL_SCANCODE_MEDIA_REWIND=266,
	SDL_SCANCODE_MEDIA_NEXT_TRACK=267,
	SDL_SCANCODE_MEDIA_PREVIOUS_TRACK=268,
	SDL_SCANCODE_MEDIA_STOP=269,
	SDL_SCANCODE_MEDIA_EJECT=270,
	SDL_SCANCODE_MEDIA_PLAY_PAUSE=271,
	SDL_SCANCODE_MEDIA_SELECT=272,
	SDL_SCANCODE_AC_NEW=273,
	SDL_SCANCODE_AC_OPEN=274,
	SDL_SCANCODE_AC_CLOSE=275,
	SDL_SCANCODE_AC_EXIT=276,
	SDL_SCANCODE_AC_SAVE=277,
	SDL_SCANCODE_AC_PRINT=278,
	SDL_SCANCODE_AC_PROPERTIES=279,
	SDL_SCANCODE_AC_SEARCH=280,
	SDL_SCANCODE_AC_HOME=281,
	SDL_SCANCODE_AC_BACK=282,
	SDL_SCANCODE_AC_FORWARD=283,
	SDL_SCANCODE_AC_STOP=284,
	SDL_SCANCODE_AC_REFRESH=285,
	SDL_SCANCODE_AC_BOOKMARKS=286,
	SDL_SCANCODE_SOFTLEFT=287,
	SDL_SCANCODE_SOFTRIGHT=288,
	SDL_SCANCODE_CALL=289,
	SDL_SCANCODE_ENDCALL=290,
	SDL_SCANCODE_RESERVED=400,
	SDL_SCANCODE_COUNT=512,
};
typedef Uint32 SDL_Keycode;
typedef u16 Uint16;
typedef Uint16 SDL_Keymod;
struct SDL_KeyboardEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_KeyboardID which;
	enum SDL_Scancode scancode;
	SDL_Keycode key;
	SDL_Keymod mod;
	Uint16 raw;
	bool down;
	bool repeat;
};
struct SDL_TextEditingEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	i8* text;
	Sint32 start;
	Sint32 length;
};
struct SDL_TextEditingCandidatesEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	i8** candidates;
	Sint32 num_candidates;
	Sint32 selected_candidate;
	bool horizontal;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
};
struct SDL_TextInputEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	i8* text;
};
typedef Uint32 SDL_MouseID;
struct SDL_MouseDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_MouseID which;
};
typedef Uint32 SDL_MouseButtonFlags;
struct SDL_MouseMotionEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_MouseID which;
	SDL_MouseButtonFlags state;
	float x;
	float y;
	float xrel;
	float yrel;
};
struct SDL_MouseButtonEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_MouseID which;
	Uint8 button;
	bool down;
	Uint8 clicks;
	Uint8 padding;
	float x;
	float y;
};
enum SDL_MouseWheelDirection:i32{
	SDL_MOUSEWHEEL_NORMAL,
	SDL_MOUSEWHEEL_FLIPPED,
};
struct SDL_MouseWheelEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_MouseID which;
	float x;
	float y;
	enum SDL_MouseWheelDirection direction;
	float mouse_x;
	float mouse_y;
	Sint32 integer_x;
	Sint32 integer_y;
};
typedef Uint32 SDL_JoystickID;
struct SDL_JoyDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
};
typedef i16 Sint16;
struct SDL_JoyAxisEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 axis;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint16 value;
	Uint16 padding4;
};
struct SDL_JoyBallEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 ball;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint16 xrel;
	Sint16 yrel;
};
struct SDL_JoyHatEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 hat;
	Uint8 value;
	Uint8 padding1;
	Uint8 padding2;
};
struct SDL_JoyButtonEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 button;
	bool down;
	Uint8 padding1;
	Uint8 padding2;
};
enum SDL_PowerState:i32{
	SDL_POWERSTATE_ERROR=1,
	SDL_POWERSTATE_UNKNOWN,
	SDL_POWERSTATE_ON_BATTERY,
	SDL_POWERSTATE_NO_BATTERY,
	SDL_POWERSTATE_CHARGING,
	SDL_POWERSTATE_CHARGED,
};
struct SDL_JoyBatteryEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	enum SDL_PowerState state;
	i32 percent;
};
struct SDL_GamepadDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
};
struct SDL_GamepadAxisEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 axis;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint16 value;
	Uint16 padding4;
};
struct SDL_GamepadButtonEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 button;
	bool down;
	Uint8 padding1;
	Uint8 padding2;
};
struct SDL_GamepadTouchpadEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Sint32 touchpad;
	Sint32 finger;
	float x;
	float y;
	float pressure;
};
typedef struct{float data[3];}A3f;
struct SDL_GamepadSensorEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Sint32 sensor;
	A3f data;
	Uint64 sensor_timestamp;
};
struct SDL_GamepadCapSenseEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 capsense;
	bool down;
	Uint8 padding1;
	Uint8 padding2;
};
typedef Uint32 SDL_AudioDeviceID;
struct SDL_AudioDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_AudioDeviceID which;
	bool recording;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
};
typedef Uint32 SDL_CameraID;
struct SDL_CameraDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_CameraID which;
};
typedef Uint32 SDL_SensorID;
typedef struct{float data[6];}A6f;
struct SDL_SensorEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_SensorID which;
	A6f data;
	Uint64 sensor_timestamp;
};
struct SDL_QuitEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
};
struct SDL_UserEvent{
	Uint32 type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	Sint32 code;
	u8* data1;
	u8* data2;
};
typedef Uint64 SDL_TouchID;
typedef Uint64 SDL_FingerID;
struct SDL_TouchFingerEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_TouchID touchID;
	SDL_FingerID fingerID;
	float x;
	float y;
	float dx;
	float dy;
	float pressure;
	SDL_WindowID windowID;
};
struct SDL_PinchFingerEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	float scale;
	SDL_WindowID windowID;
};
typedef Uint32 SDL_PenID;
struct SDL_PenProximityEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
};
typedef Uint32 SDL_PenInputFlags;
struct SDL_PenTouchEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
	SDL_PenInputFlags pen_state;
	float x;
	float y;
	bool eraser;
	bool down;
};
struct SDL_PenMotionEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
	SDL_PenInputFlags pen_state;
	float x;
	float y;
};
struct SDL_PenButtonEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
	SDL_PenInputFlags pen_state;
	float x;
	float y;
	Uint8 button;
	bool down;
};
enum SDL_PenAxis:i32{
	SDL_PEN_AXIS_PRESSURE,
	SDL_PEN_AXIS_XTILT,
	SDL_PEN_AXIS_YTILT,
	SDL_PEN_AXIS_DISTANCE,
	SDL_PEN_AXIS_ROTATION,
	SDL_PEN_AXIS_SLIDER,
	SDL_PEN_AXIS_TANGENTIAL_PRESSURE,
	SDL_PEN_AXIS_COUNT,
};
struct SDL_PenAxisEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
	SDL_PenInputFlags pen_state;
	float x;
	float y;
	enum SDL_PenAxis axis;
	float value;
};
struct SDL_RenderEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
};
struct SDL_DropEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	float x;
	float y;
	i8* source;
	i8* data;
};
struct SDL_ClipboardEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	bool owner;
	Sint32 num_mime_types;
	i8** mime_types;
};
typedef struct{Uint8 data[128];}A128tUint8;
union SDL_Event{
	Uint32 type;
	struct SDL_CommonEvent common;
	struct SDL_DisplayEvent display;
	struct SDL_WindowEvent window;
	struct SDL_KeyboardDeviceEvent kdevice;
	struct SDL_KeyboardEvent key;
	struct SDL_TextEditingEvent edit;
	struct SDL_TextEditingCandidatesEvent edit_candidates;
	struct SDL_TextInputEvent text;
	struct SDL_MouseDeviceEvent mdevice;
	struct SDL_MouseMotionEvent motion;
	struct SDL_MouseButtonEvent button;
	struct SDL_MouseWheelEvent wheel;
	struct SDL_JoyDeviceEvent jdevice;
	struct SDL_JoyAxisEvent jaxis;
	struct SDL_JoyBallEvent jball;
	struct SDL_JoyHatEvent jhat;
	struct SDL_JoyButtonEvent jbutton;
	struct SDL_JoyBatteryEvent jbattery;
	struct SDL_GamepadDeviceEvent gdevice;
	struct SDL_GamepadAxisEvent gaxis;
	struct SDL_GamepadButtonEvent gbutton;
	struct SDL_GamepadTouchpadEvent gtouchpad;
	struct SDL_GamepadSensorEvent gsensor;
	struct SDL_GamepadCapSenseEvent gcapsense;
	struct SDL_AudioDeviceEvent adevice;
	struct SDL_CameraDeviceEvent cdevice;
	struct SDL_SensorEvent sensor;
	struct SDL_QuitEvent quit;
	struct SDL_UserEvent user;
	struct SDL_TouchFingerEvent tfinger;
	struct SDL_PinchFingerEvent pinch;
	struct SDL_PenProximityEvent pproximity;
	struct SDL_PenTouchEvent ptouch;
	struct SDL_PenMotionEvent pmotion;
	struct SDL_PenButtonEvent pbutton;
	struct SDL_PenAxisEvent paxis;
	struct SDL_RenderEvent render;
	struct SDL_DropEvent drop;
	struct SDL_ClipboardEvent clipboard;
	A128tUint8 padding;
};
struct SDL_GPUTexture;
struct SDL_FColor{
	float r;
	float g;
	float b;
	float a;
};
enum SDL_GPULoadOp:i32{
	SDL_GPU_LOADOP_LOAD,
	SDL_GPU_LOADOP_CLEAR,
	SDL_GPU_LOADOP_DONT_CARE,
};
enum SDL_GPUStoreOp:i32{
	SDL_GPU_STOREOP_STORE,
	SDL_GPU_STOREOP_DONT_CARE,
	SDL_GPU_STOREOP_RESOLVE,
	SDL_GPU_STOREOP_RESOLVE_AND_STORE,
};
struct SDL_GPUColorTargetInfo{
	struct SDL_GPUTexture* texture;
	Uint32 mip_level;
	Uint32 layer_or_depth_plane;
	struct SDL_FColor clear_color;
	enum SDL_GPULoadOp load_op;
	enum SDL_GPUStoreOp store_op;
	struct SDL_GPUTexture* resolve_texture;
	Uint32 resolve_mip_level;
	Uint32 resolve_layer;
	bool cycle;
	bool cycle_resolve_texture;
	Uint8 padding1;
	Uint8 padding2;
};
struct SDL_GPURenderPass;
struct SDL_GPUDepthStencilTargetInfo;
struct SDL_GPUBufferBinding{
	struct SDL_GPUBuffer* buffer;
	Uint32 offset;
};
typedef struct{struct SDL_GPUBufferBinding data[1];}A1SSDL_GPUBufferBinding;


struct SDL_GPUShader* _loadShader_F3stESDL_GPUShaderStagepSSDL_GPUDevice_pSSDL_GPUShader(string path,enum SDL_GPUShaderStage stage,struct SDL_GPUDevice* device);
DLLIMPORT extern void* SDL_LoadFile(i8* file,u64* datasize);
DLLIMPORT extern struct SDL_GPUShader* SDL_CreateGPUShader(struct SDL_GPUDevice* device,struct SDL_GPUShaderCreateInfo* createinfo);
DLLIMPORT extern void SDL_free(u8* mem);
struct SDL_GPUColorTargetBlendState _createOpaqueBlendState_F0_SSDL_GPUColorTargetBlendState();
void _main_F0();
DLLIMPORT extern bool SDL_Init(SDL_InitFlags flags);
DLLIMPORT extern struct SDL_Window* SDL_CreateWindow(i8* title,i32 w,i32 h,SDL_WindowFlags flags);
DLLIMPORT extern struct SDL_GPUDevice* SDL_CreateGPUDevice(SDL_GPUShaderFormat format_flags,bool debug_mode,i8* name);
DLLIMPORT extern bool SDL_ClaimWindowForGPUDevice(struct SDL_GPUDevice* device,struct SDL_Window* window);
DLLIMPORT extern struct SDL_GPUBuffer* SDL_CreateGPUBuffer(struct SDL_GPUDevice* device,struct SDL_GPUBufferCreateInfo* createinfo);
DLLIMPORT extern struct SDL_GPUTransferBuffer* SDL_CreateGPUTransferBuffer(struct SDL_GPUDevice* device,struct SDL_GPUTransferBufferCreateInfo* createinfo);
DLLIMPORT extern void* SDL_MapGPUTransferBuffer(struct SDL_GPUDevice* device,struct SDL_GPUTransferBuffer* transfer_buffer,bool cycle);
DLLIMPORT extern void SDL_UnmapGPUTransferBuffer(struct SDL_GPUDevice* device,struct SDL_GPUTransferBuffer* transfer_buffer);
DLLIMPORT extern struct SDL_GPUCommandBuffer* SDL_AcquireGPUCommandBuffer(struct SDL_GPUDevice* device);
DLLIMPORT extern struct SDL_GPUCopyPass* SDL_BeginGPUCopyPass(struct SDL_GPUCommandBuffer* command_buffer);
DLLIMPORT extern void SDL_UploadToGPUBuffer(struct SDL_GPUCopyPass* copy_pass,struct SDL_GPUTransferBufferLocation* source,struct SDL_GPUBufferRegion* destination,bool cycle);
DLLIMPORT extern void SDL_EndGPUCopyPass(struct SDL_GPUCopyPass* copy_pass);
DLLIMPORT extern bool SDL_SubmitGPUCommandBuffer(struct SDL_GPUCommandBuffer* command_buffer);
DLLIMPORT extern enum SDL_GPUTextureFormat SDL_GetGPUSwapchainTextureFormat(struct SDL_GPUDevice* device,struct SDL_Window* window);
DLLIMPORT extern struct SDL_GPUGraphicsPipeline* SDL_CreateGPUGraphicsPipeline(struct SDL_GPUDevice* device,struct SDL_GPUGraphicsPipelineCreateInfo* createinfo);
DLLIMPORT extern void SDL_ReleaseGPUShader(struct SDL_GPUDevice* device,struct SDL_GPUShader* shader);
DLLIMPORT extern bool SDL_PollEvent(union SDL_Event* event);
DLLIMPORT extern bool SDL_WaitAndAcquireGPUSwapchainTexture(struct SDL_GPUCommandBuffer* command_buffer,struct SDL_Window* window,struct SDL_GPUTexture** swapchain_texture,Uint32* swapchain_texture_width,Uint32* swapchain_texture_height);
DLLIMPORT extern struct SDL_GPURenderPass* SDL_BeginGPURenderPass(struct SDL_GPUCommandBuffer* command_buffer,struct SDL_GPUColorTargetInfo* color_target_infos,Uint32 num_color_targets,struct SDL_GPUDepthStencilTargetInfo* depth_stencil_target_info);
DLLIMPORT extern void SDL_BindGPUGraphicsPipeline(struct SDL_GPURenderPass* render_pass,struct SDL_GPUGraphicsPipeline* graphics_pipeline);
DLLIMPORT extern void SDL_BindGPUVertexBuffers(struct SDL_GPURenderPass* render_pass,Uint32 first_slot,struct SDL_GPUBufferBinding* bindings,Uint32 num_bindings);
DLLIMPORT extern void SDL_DrawGPUPrimitives(struct SDL_GPURenderPass* render_pass,Uint32 num_vertices,Uint32 num_instances,Uint32 first_vertex,Uint32 first_instance);
DLLIMPORT extern void SDL_EndGPURenderPass(struct SDL_GPURenderPass* render_pass);
DLLIMPORT extern void SDL_ReleaseGPUGraphicsPipeline(struct SDL_GPUDevice* device,struct SDL_GPUGraphicsPipeline* graphics_pipeline);
DLLIMPORT extern void SDL_ReleaseGPUBuffer(struct SDL_GPUDevice* device,struct SDL_GPUBuffer* buffer);
DLLIMPORT extern void SDL_ReleaseGPUTransferBuffer(struct SDL_GPUDevice* device,struct SDL_GPUTransferBuffer* transfer_buffer);
DLLIMPORT extern void SDL_DestroyGPUDevice(struct SDL_GPUDevice* device);
DLLIMPORT extern void SDL_DestroyWindow(struct SDL_Window* window);
DLLIMPORT extern void SDL_Quit();


const A3SVertex vertices=(A3SVertex){(struct Vertex){(struct vec3){0.0,0.5,0},(struct vec4){1,0,0,1}},(struct Vertex){(struct vec3){-0.5,-0.5,0},(struct vec4){1,1,0,1}},(struct Vertex){(struct vec3){0.5,-0.5,0},(struct vec4){1,0,1,1}}};
static i8* const _G1="main";
static i8* const _G2="Triangle";
static i8* const _G3="res/triangle.vert.bin";
static i8* const _G4="res/triangle.frag.bin";


struct SDL_GPUShader* _loadShader_F3stESDL_GPUShaderStagepSSDL_GPUDevice_pSSDL_GPUShader(string path,enum SDL_GPUShaderStage stage,struct SDL_GPUDevice* device){
	u64 size={0};
	i8* const _1=path.data;
	u64* const _2=&size;
	void* const _3=SDL_LoadFile(_1,_2);
	u8* const _4=(u8*)_3;
	u8* code=_4;
	const bool _5=(bool)code;
	const bool _6=!_5;
	if(_6){
		return 0;
	}
	struct SDL_GPUShaderCreateInfo shaderInfo={0};
	Uint8** const _7=&shaderInfo.code;
	Uint8* const _8=(Uint8*)code;
	(*_7)=_8;
	u64* const _9=&shaderInfo.code_size;
	(*_9)=size;
	i8** const _10=&shaderInfo.entrypoint;
	(*_10)=_G1;
	SDL_GPUShaderFormat* const _11=&shaderInfo.format;
	const u32 _12=1u<<1;
	const SDL_GPUShaderFormat _13=(SDL_GPUShaderFormat)_12;
	(*_11)=_13;
	enum SDL_GPUShaderStage* const _14=&shaderInfo.stage;
	(*_14)=stage;
	Uint32* const _15=&shaderInfo.num_samplers;
	const Uint32 _16=(Uint32)0u;
	(*_15)=_16;
	Uint32* const _17=&shaderInfo.num_storage_textures;
	const Uint32 _18=(Uint32)0u;
	(*_17)=_18;
	Uint32* const _19=&shaderInfo.num_storage_buffers;
	const Uint32 _20=(Uint32)0u;
	(*_19)=_20;
	Uint32* const _21=&shaderInfo.num_uniform_buffers;
	const Uint32 _22=(Uint32)0u;
	(*_21)=_22;
	struct SDL_GPUShaderCreateInfo* const _23=&shaderInfo;
	struct SDL_GPUShader* const _24=SDL_CreateGPUShader(device,_23);
	struct SDL_GPUShader* shader=_24;
	SDL_free(code);
	return shader;
}
struct SDL_GPUColorTargetBlendState _createOpaqueBlendState_F0_SSDL_GPUColorTargetBlendState(){
	struct SDL_GPUColorTargetBlendState blendState={0};
	bool* const _1=&blendState.enable_blend;
	(*_1)=false;
	return blendState;
}
void _main_F0(){
	const SDL_InitFlags _1=(SDL_InitFlags)0x20u;
	const bool _2=SDL_Init(_1);
	const SDL_WindowFlags _3=(SDL_WindowFlags)0llu;
	struct SDL_Window* const _4=SDL_CreateWindow(_G2,1280,720,_3);
	struct SDL_Window* window=_4;
	const u32 _5=1u<<1;
	const SDL_GPUShaderFormat _6=(SDL_GPUShaderFormat)_5;
	struct SDL_GPUDevice* const _7=SDL_CreateGPUDevice(_6,true,0);
	struct SDL_GPUDevice* device=_7;
	const bool _8=SDL_ClaimWindowForGPUDevice(device,window);
	struct SDL_GPUBufferCreateInfo bufferInfo={0};
	Uint32* const _9=&bufferInfo.size;
	const u64 _10=(u64)sizeof(A3SVertex);
	const u32 _11=(u32)_10;
	const Uint32 _12=(Uint32)_11;
	(*_9)=_12;
	SDL_GPUBufferUsageFlags* const _13=&bufferInfo.usage;
	const u32 _14=1u<<0;
	const SDL_GPUBufferUsageFlags _15=(SDL_GPUBufferUsageFlags)_14;
	(*_13)=_15;
	struct SDL_GPUBufferCreateInfo* const _16=&bufferInfo;
	struct SDL_GPUBuffer* const _17=SDL_CreateGPUBuffer(device,_16);
	struct SDL_GPUBuffer* vertexBuffer=_17;
	struct SDL_GPUTransferBufferCreateInfo transferBufferInfo={0};
	Uint32* const _18=&transferBufferInfo.size;
	const u64 _19=(u64)sizeof(A3SVertex);
	const u32 _20=(u32)_19;
	const Uint32 _21=(Uint32)_20;
	(*_18)=_21;
	enum SDL_GPUTransferBufferUsage* const _22=&transferBufferInfo.usage;
	(*_22)=0;
	struct SDL_GPUTransferBufferCreateInfo* const _23=&transferBufferInfo;
	struct SDL_GPUTransferBuffer* const _24=SDL_CreateGPUTransferBuffer(device,_23);
	struct SDL_GPUTransferBuffer* transferBuffer=_24;
	void* const _25=SDL_MapGPUTransferBuffer(device,transferBuffer,false);
	struct Vertex* const _26=(struct Vertex*)_25;
	struct Vertex* vertexData=_26;
	const struct vec3 _27={0.0,0.5,0};
	const struct vec4 _28={1,0,0,1};
	const struct Vertex _29={_27,_28};
	const struct vec3 _30={-0.5,-0.5,0};
	const struct vec4 _31={1,1,0,1};
	const struct Vertex _32={_30,_31};
	const struct vec3 _33={0.5,-0.5,0};
	const struct vec4 _34={1,0,1,1};
	const struct Vertex _35={_33,_34};
	const A3SVertex _36={_29,_32,_35};
	const i32 _37=3>=0?1:-1;
	for(int i=0;i*_37<3*_37;i+=_37){
		struct Vertex* const _38=&vertexData[i];
		const struct vec3 _39={0.0,0.5,0};
		const struct vec4 _40={1,0,0,1};
		const struct Vertex _41={_39,_40};
		const struct vec3 _42={-0.5,-0.5,0};
		const struct vec4 _43={1,1,0,1};
		const struct Vertex _44={_42,_43};
		const struct vec3 _45={0.5,-0.5,0};
		const struct vec4 _46={1,0,1,1};
		const struct Vertex _47={_45,_46};
		const A3SVertex _48={_41,_44,_47};
		struct Vertex* const _49=&_48.data[i];
		(*_38)=(*_49);
	}
	SDL_UnmapGPUTransferBuffer(device,transferBuffer);
	struct SDL_GPUCommandBuffer* const _50=SDL_AcquireGPUCommandBuffer(device);
	struct SDL_GPUCommandBuffer* cmdBuffer=_50;
	struct SDL_GPUCopyPass* const _51=SDL_BeginGPUCopyPass(cmdBuffer);
	struct SDL_GPUCopyPass* copyPass=_51;
	struct SDL_GPUTransferBufferLocation src={0};
	struct SDL_GPUTransferBuffer** const _52=&src.transfer_buffer;
	(*_52)=transferBuffer;
	struct SDL_GPUBufferRegion dst={0};
	struct SDL_GPUBuffer** const _53=&dst.buffer;
	(*_53)=vertexBuffer;
	Uint32* const _54=&dst.size;
	const u64 _55=(u64)sizeof(A3SVertex);
	const u32 _56=(u32)_55;
	const Uint32 _57=(Uint32)_56;
	(*_54)=_57;
	struct SDL_GPUTransferBufferLocation* const _58=&src;
	struct SDL_GPUBufferRegion* const _59=&dst;
	SDL_UploadToGPUBuffer(copyPass,_58,_59,true);
	SDL_EndGPUCopyPass(copyPass);
	const bool _60=SDL_SubmitGPUCommandBuffer(cmdBuffer);
	const string _61={_G3,21};
	struct SDL_GPUShader* const _62=_loadShader_F3stESDL_GPUShaderStagepSSDL_GPUDevice_pSSDL_GPUShader(_61,0,device);
	struct SDL_GPUShader* vertexShader=_62;
	const string _63={_G4,21};
	struct SDL_GPUShader* const _64=_loadShader_F3stESDL_GPUShaderStagepSSDL_GPUDevice_pSSDL_GPUShader(_63,1,device);
	struct SDL_GPUShader* fragmentShader=_64;
	A1SSDL_GPUVertexBufferDescription vertexBufferDescriptions={0};
	struct SDL_GPUVertexBufferDescription* const _65=&vertexBufferDescriptions.data[0];
	Uint32* const _66=&(*_65).slot;
	const Uint32 _67=(Uint32)0u;
	(*_66)=_67;
	struct SDL_GPUVertexBufferDescription* const _68=&vertexBufferDescriptions.data[0];
	enum SDL_GPUVertexInputRate* const _69=&(*_68).input_rate;
	(*_69)=0;
	struct SDL_GPUVertexBufferDescription* const _70=&vertexBufferDescriptions.data[0];
	Uint32* const _71=&(*_70).instance_step_rate;
	const Uint32 _72=(Uint32)0u;
	(*_71)=_72;
	struct SDL_GPUVertexBufferDescription* const _73=&vertexBufferDescriptions.data[0];
	Uint32* const _74=&(*_73).pitch;
	const u64 _75=(u64)sizeof(struct Vertex);
	const u32 _76=(u32)_75;
	const Uint32 _77=(Uint32)_76;
	(*_74)=_77;
	A2SSDL_GPUVertexAttribute attributes={0};
	struct SDL_GPUVertexAttribute* const _78=&attributes.data[0];
	Uint32* const _79=&(*_78).buffer_slot;
	const Uint32 _80=(Uint32)0u;
	(*_79)=_80;
	struct SDL_GPUVertexAttribute* const _81=&attributes.data[0];
	Uint32* const _82=&(*_81).location;
	const Uint32 _83=(Uint32)0u;
	(*_82)=_83;
	struct SDL_GPUVertexAttribute* const _84=&attributes.data[0];
	enum SDL_GPUVertexElementFormat* const _85=&(*_84).format;
	(*_85)=11;
	struct SDL_GPUVertexAttribute* const _86=&attributes.data[0];
	Uint32* const _87=&(*_86).offset;
	const Uint32 _88=(Uint32)0u;
	(*_87)=_88;
	struct SDL_GPUVertexAttribute* const _89=&attributes.data[1];
	Uint32* const _90=&(*_89).buffer_slot;
	const Uint32 _91=(Uint32)0u;
	(*_90)=_91;
	struct SDL_GPUVertexAttribute* const _92=&attributes.data[1];
	Uint32* const _93=&(*_92).location;
	const Uint32 _94=(Uint32)1u;
	(*_93)=_94;
	struct SDL_GPUVertexAttribute* const _95=&attributes.data[1];
	enum SDL_GPUVertexElementFormat* const _96=&(*_95).format;
	(*_96)=12;
	struct SDL_GPUVertexAttribute* const _97=&attributes.data[1];
	Uint32* const _98=&(*_97).offset;
	const u64 _99=(u64)sizeof(struct vec3);
	const u32 _100=(u32)_99;
	const Uint32 _101=(Uint32)_100;
	(*_98)=_101;
	A1SSDL_GPUColorTargetDescription targets={0};
	struct SDL_GPUColorTargetDescription* const _102=&targets.data[0];
	enum SDL_GPUTextureFormat* const _103=&(*_102).format;
	const enum SDL_GPUTextureFormat _104=SDL_GetGPUSwapchainTextureFormat(device,window);
	(*_103)=_104;
	struct SDL_GPUColorTargetDescription* const _105=&targets.data[0];
	struct SDL_GPUColorTargetBlendState* const _106=&(*_105).blend_state;
	const struct SDL_GPUColorTargetBlendState _107=_createOpaqueBlendState_F0_SSDL_GPUColorTargetBlendState();
	(*_106)=_107;
	struct SDL_GPUGraphicsPipelineCreateInfo pipelineInfo={0};
	struct SDL_GPUShader** const _108=&pipelineInfo.vertex_shader;
	(*_108)=vertexShader;
	struct SDL_GPUShader** const _109=&pipelineInfo.fragment_shader;
	(*_109)=fragmentShader;
	enum SDL_GPUPrimitiveType* const _110=&pipelineInfo.primitive_type;
	(*_110)=0;
	struct SDL_GPUVertexInputState* const _111=&pipelineInfo.vertex_input_state;
	Uint32* const _112=&(*_111).num_vertex_buffers;
	const Uint32 _113=(Uint32)1u;
	(*_112)=_113;
	struct SDL_GPUVertexInputState* const _114=&pipelineInfo.vertex_input_state;
	struct SDL_GPUVertexBufferDescription** const _115=&(*_114).vertex_buffer_descriptions;
	struct SDL_GPUVertexBufferDescription* const _116=&vertexBufferDescriptions.data[0];
	struct SDL_GPUVertexBufferDescription* const _117=&(*_116);
	(*_115)=_117;
	struct SDL_GPUVertexInputState* const _118=&pipelineInfo.vertex_input_state;
	Uint32* const _119=&(*_118).num_vertex_attributes;
	const Uint32 _120=(Uint32)2u;
	(*_119)=_120;
	struct SDL_GPUVertexInputState* const _121=&pipelineInfo.vertex_input_state;
	struct SDL_GPUVertexAttribute** const _122=&(*_121).vertex_attributes;
	struct SDL_GPUVertexAttribute* const _123=&attributes.data[0];
	struct SDL_GPUVertexAttribute* const _124=&(*_123);
	(*_122)=_124;
	struct SDL_GPUGraphicsPipelineTargetInfo* const _125=&pipelineInfo.target_info;
	Uint32* const _126=&(*_125).num_color_targets;
	const Uint32 _127=(Uint32)1u;
	(*_126)=_127;
	struct SDL_GPUGraphicsPipelineTargetInfo* const _128=&pipelineInfo.target_info;
	struct SDL_GPUColorTargetDescription** const _129=&(*_128).color_target_descriptions;
	struct SDL_GPUColorTargetDescription* const _130=&targets.data[0];
	struct SDL_GPUColorTargetDescription* const _131=&(*_130);
	(*_129)=_131;
	struct SDL_GPUGraphicsPipelineCreateInfo* const _132=&pipelineInfo;
	struct SDL_GPUGraphicsPipeline* const _133=SDL_CreateGPUGraphicsPipeline(device,_132);
	struct SDL_GPUGraphicsPipeline* pipeline=_133;
	SDL_ReleaseGPUShader(device,vertexShader);
	SDL_ReleaseGPUShader(device,fragmentShader);
	bool running=true;
	while(1){
		if(!running)break;
		union SDL_Event event={0};
		while(1){
			union SDL_Event* const _134=&event;
			const bool _135=SDL_PollEvent(_134);
			if(!_135)break;
			Uint32* const _136=&event.type;
			const enum SDL_EventType _137=(enum SDL_EventType)(*_136);
			const bool _138=_137==256;
			if(_138){
				running=false;
			}
		}
		struct SDL_GPUCommandBuffer* const _139=SDL_AcquireGPUCommandBuffer(device);
		struct SDL_GPUCommandBuffer* cmdBuffer=_139;
		struct SDL_GPUTexture* swapchain={0};
		u32 width={0};
		u32 height={0};
		struct SDL_GPUTexture** const _140=&swapchain;
		u32* const _141=&width;
		Uint32* const _142=(Uint32*)_141;
		u32* const _143=&height;
		Uint32* const _144=(Uint32*)_143;
		const bool _145=SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuffer,window,_140,_142,_144);
		struct SDL_GPUColorTargetInfo colorTarget={0};
		struct SDL_FColor* const _146=&colorTarget.clear_color;
		const struct SDL_FColor _147={0.8,0.8,0.8,1.0};
		(*_146)=_147;
		enum SDL_GPULoadOp* const _148=&colorTarget.load_op;
		(*_148)=1;
		enum SDL_GPUStoreOp* const _149=&colorTarget.store_op;
		(*_149)=0;
		struct SDL_GPUTexture** const _150=&colorTarget.texture;
		(*_150)=swapchain;
		struct SDL_GPUColorTargetInfo* const _151=&colorTarget;
		const Uint32 _152=(Uint32)1u;
		struct SDL_GPURenderPass* const _153=SDL_BeginGPURenderPass(cmdBuffer,_151,_152,0);
		struct SDL_GPURenderPass* renderPass=_153;
		SDL_BindGPUGraphicsPipeline(renderPass,pipeline);
		A1SSDL_GPUBufferBinding bufferBindings={0};
		struct SDL_GPUBufferBinding* const _154=&bufferBindings.data[0];
		struct SDL_GPUBuffer** const _155=&(*_154).buffer;
		(*_155)=vertexBuffer;
		struct SDL_GPUBufferBinding* const _156=&bufferBindings.data[0];
		Uint32* const _157=&(*_156).offset;
		const Uint32 _158=(Uint32)0u;
		(*_157)=_158;
		const Uint32 _159=(Uint32)0u;
		struct SDL_GPUBufferBinding* const _160=bufferBindings.data;
		const Uint32 _161=(Uint32)1u;
		SDL_BindGPUVertexBuffers(renderPass,_159,_160,_161);
		const Uint32 _162=(Uint32)3u;
		const Uint32 _163=(Uint32)1u;
		const Uint32 _164=(Uint32)0u;
		const Uint32 _165=(Uint32)0u;
		SDL_DrawGPUPrimitives(renderPass,_162,_163,_164,_165);
		SDL_EndGPURenderPass(renderPass);
		const bool _166=SDL_SubmitGPUCommandBuffer(cmdBuffer);
	}
	SDL_ReleaseGPUGraphicsPipeline(device,pipeline);
	SDL_ReleaseGPUBuffer(device,vertexBuffer);
	SDL_ReleaseGPUTransferBuffer(device,transferBuffer);
	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
int main(int argc, char* argv[]){
	_main_F0();
	return 0;
}