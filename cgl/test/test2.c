// raylib
#include <cgl.h>


struct Vector2{
        float x;
        float y;
};

struct Vector3{
        float x;
        float y;
        float z;
};

struct Vector4{
        float x;
        float y;
        float z;
        float w;
};

struct Matrix{
        float m0;
        float m4;
        float m8;
        float m12;
        float m1;
        float m5;
        float m9;
        float m13;
        float m2;
        float m6;
        float m10;
        float m14;
        float m3;
        float m7;
        float m11;
        float m15;
};

struct Color{
        u8 r;
        u8 g;
        u8 b;
        u8 a;
};

struct Rectangle{
        float x;
        float y;
        float width;
        float height;
};

struct Image{
        u8* data;
        i32 width;
        i32 height;
        i32 mipmaps;
        i32 format;
};

struct Texture{
        u32 id;
        i32 width;
        i32 height;
        i32 mipmaps;
        i32 format;
};

struct RenderTexture{
        u32 id;
        struct Texture texture;
        struct Texture depth;
};

struct NPatchInfo{
        struct Rectangle source;
        i32 left;
        i32 top;
        i32 right;
        i32 bottom;
        i32 layout;
};

struct GlyphInfo{
        i32 value;
        i32 offsetX;
        i32 offsetY;
        i32 advanceX;
        struct Image image;
};

struct Font{
        i32 baseSize;
        i32 glyphCount;
        i32 glyphPadding;
        struct Texture texture;
        struct Rectangle* rects;
        struct GlyphInfo* glyphs;
};

struct Camera3D{
        struct Vector3 position;
        struct Vector3 target;
        struct Vector3 up;
        float fovy;
        i32 projection;
};

struct Camera2D{
        struct Vector2 offset;
        struct Vector2 target;
        float rotation;
        float zoom;
};

struct Mesh{
        i32 vertexCount;
        i32 triangleCount;
        float* vertices;
        float* texcoords;
        float* texcoords2;
        float* normals;
        float* tangents;
        u8* colors;
        u16* indices;
        float* animVertices;
        float* animNormals;
        u8* boneIds;
        float* boneWeights;
        u32 vaoId;
        u32* vboId;
};

struct Shader{
        u32 id;
        i32* locs;
};

struct MaterialMap{
        struct Texture texture;
        struct Color color;
        float value;
};

typedef struct{float buffer[4];}__a4f32;
struct Material{
        struct Shader shader;
        struct MaterialMap* maps;
        __a4f32 params;
};

struct Transform{
        struct Vector3 translation;
        struct Vector4 rotation;
        struct Vector3 scale;
};

typedef struct{i8 buffer[32];}__a32i8;
struct BoneInfo{
        __a32i8 name;
        i32 parent;
};

struct Model{
        struct Matrix transform;
        i32 meshCount;
        i32 materialCount;
        struct Mesh* meshes;
        struct Material* materials;
        i32* meshMaterial;
        i32 boneCount;
        struct BoneInfo* bones;
        struct Transform* bindPose;
};

struct ModelAnimation{
        i32 boneCount;
        i32 frameCount;
        struct BoneInfo* bones;
        struct Transform** framePoses;
        __a32i8 name;
};

struct Ray{
        struct Vector3 position;
        struct Vector3 direction;
};

struct RayCollision{
        bool hit;
        float distance;
        struct Vector3 point;
        struct Vector3 normal;
};

struct BoundingBox{
        struct Vector3 min;
        struct Vector3 max;
};

struct Wave{
        u32 frameCount;
        u32 sampleRate;
        u32 sampleSize;
        u32 channels;
        u8* data;
};

struct AudioStream{
        void* buffer;
        void* processor;
        u32 sampleRate;
        u32 sampleSize;
        u32 channels;
};

struct Sound{
        struct AudioStream stream;
        u32 frameCount;
};

struct Music{
        struct AudioStream stream;
        u32 frameCount;
        bool looping;
        i32 ctxType;
        u8* ctxData;
};

struct VrDeviceInfo{
        i32 hResolution;
        i32 vResolution;
        float hScreenSize;
        float vScreenSize;
        float vScreenCenter;
        float eyeToScreenDistance;
        float lensSeparationDistance;
        float interpupillaryDistance;
        __a4f32 lensDistortionValues;
        __a4f32 chromaAbCorrection;
};

typedef struct{struct Matrix buffer[2];}__a2xMatrix;
typedef struct{float buffer[2];}__a2f32;
struct VrStereoConfig{
        __a2xMatrix projection;
        __a2xMatrix viewOffset;
        __a2f32 leftLensCenter;
        __a2f32 rightLensCenter;
        __a2f32 leftScreenCenter;
        __a2f32 rightScreenCenter;
        __a2f32 scale;
        __a2f32 scaleIn;
};

struct FilePathList{
        u32 capacity;
        u32 count;
        u8** paths;
};

typedef struct{i32 buffer[4];}__a4i32;
struct AutomationEvent{
        u32 frame;
        u32 type;
        __a4i32 params;
};

struct AutomationEventList{
        u32 capacity;
        u32 count;
        struct AutomationEvent* events;
};

typedef void(*__Fv3i32pi8pi8)(i32,i8*,i8*);
typedef i8*(*__Fpi82pi8pi32)(i8*,i32*);
typedef bool(*__Fb3pi8pu8i32)(i8*,u8*,i32);
typedef i8*(*__Fpi81pi8)(i8*);
typedef bool(*__Fb2pi8pi8)(i8*,i8*);
typedef void(*__Fv2pu8u32)(u8*,u32);
typedef struct Vector4 Quaternion;typedef struct Texture Texture2D;typedef struct Texture TextureCubemap;typedef struct RenderTexture RenderTexture2D;typedef struct Camera3D Camera;typedef void rAudioBuffer;typedef void rAudioProcessor;typedef __Fv3i32pi8pi8 TraceLogCallback;typedef __Fpi82pi8pi32 LoadFileDataCallback;typedef __Fb3pi8pu8i32 SaveFileDataCallback;typedef __Fpi81pi8 LoadFileTextCallback;typedef __Fb2pi8pi8 SaveFileTextCallback;typedef __Fv2pu8u32 AudioCallback;

#define raylib__PI (3.1415926538979323846f)
#define raylib__DEG2RAD (raylib__PI/180.0f)
#define raylib__RAD2DEG (180.0f/raylib__PI)


__declspec(dllimport) void InitWindow(i32 width,i32 height,i8* title);

extern __declspec(dllimport) void CloseWindow();

extern __declspec(dllimport) bool WindowShouldClose();

extern __declspec(dllimport) bool IsWindowReady();

extern __declspec(dllimport) bool IsWindowFullscreen();

extern __declspec(dllimport) bool IsWindowHidden();

extern __declspec(dllimport) bool IsWindowMinimized();

extern __declspec(dllimport) bool IsWindowMaximized();

extern __declspec(dllimport) bool IsWindowFocused();

extern __declspec(dllimport) bool IsWindowResized();

extern __declspec(dllimport) bool IsWindowState(u32 flag);

extern __declspec(dllimport) void SetWindowState(u32 flags);

extern __declspec(dllimport) void ClearWindowState(u32 flags);

extern __declspec(dllimport) void ToggleFullscreen();

extern __declspec(dllimport) void ToggleBorderlessWindowed();

extern __declspec(dllimport) void MaximizeWindow();

extern __declspec(dllimport) void MinimizeWindow();

extern __declspec(dllimport) void RestoreWindow();

extern __declspec(dllimport) void SetWindowIcon(struct Image image);

extern __declspec(dllimport) void SetWindowIcons(struct Image* images,i32 count);

extern __declspec(dllimport) void SetWindowTitle(i8* title);

extern __declspec(dllimport) void SetWindowPosition(i32 x,i32 y);

extern __declspec(dllimport) void SetWindowMonitor(i32 monitor);

extern __declspec(dllimport) void SetWindowMinSize(i32 width,i32 height);

extern __declspec(dllimport) void SetWindowMaxSize(i32 width,i32 height);

extern __declspec(dllimport) void SetWindowSize(i32 width,i32 height);

extern __declspec(dllimport) void SetWindowOpacity(float opacity);

extern __declspec(dllimport) void SetWindowFocused();

extern __declspec(dllimport) u8* GetWindowHandle();

extern __declspec(dllimport) i32 GetScreenWidth();

extern __declspec(dllimport) i32 GetScreenHeight();

extern __declspec(dllimport) i32 GetRenderWidth();

extern __declspec(dllimport) i32 GetRenderHeight();

extern __declspec(dllimport) i32 GetMonitorCount();

extern __declspec(dllimport) i32 GetCurrentMonitor();

extern __declspec(dllimport) struct Vector2 GetMonitorPosition(i32 monitor);

extern __declspec(dllimport) i32 GetMonitorWidth(i32 monitor);

extern __declspec(dllimport) i32 GetMonitorHeight(i32 monitor);

extern __declspec(dllimport) i32 GetMonitorPhysicalWidth(i32 monitor);

extern __declspec(dllimport) i32 GetMonitorPhysicalHeight(i32 monitor);

extern __declspec(dllimport) i32 GetMonitorRefreshRate(i32 monitor);

extern __declspec(dllimport) struct Vector2 GetWindowPosition();

extern __declspec(dllimport) struct Vector2 GetWindowScaleDPI();

extern __declspec(dllimport) i8* GetMonitorName(i32 monitor);

extern __declspec(dllimport) void SetClipboardText(i8* text);

extern __declspec(dllimport) i8* GetClipboardText();

extern __declspec(dllimport) void EnableEventWaiting();

extern __declspec(dllimport) void DisableEventWaiting();

extern __declspec(dllimport) void ShowCursor();

extern __declspec(dllimport) void HideCursor();

extern __declspec(dllimport) bool IsCursorHidden();

extern __declspec(dllimport) void EnableCursor();

extern __declspec(dllimport) void DisableCursor();

extern __declspec(dllimport) bool IsCursorOnScreen();

extern __declspec(dllimport) void ClearBackground(struct Color color);

extern __declspec(dllimport) void BeginDrawing();

extern __declspec(dllimport) void EndDrawing();

extern __declspec(dllimport) void BeginMode2D(struct Camera2D camera);

extern __declspec(dllimport) void EndMode2D();

extern __declspec(dllimport) void BeginMode3D(struct Camera3D camera);

extern __declspec(dllimport) void EndMode3D();

extern __declspec(dllimport) void BeginTextureMode(struct RenderTexture target);

extern __declspec(dllimport) void EndTextureMode();

extern __declspec(dllimport) void BeginShaderMode(struct Shader shader);

extern __declspec(dllimport) void EndShaderMode();

extern __declspec(dllimport) void BeginBlendMode(i32 mode);

extern __declspec(dllimport) void EndBlendMode();

extern __declspec(dllimport) void BeginScissorMode(i32 x,i32 y,i32 width,i32 height);

extern __declspec(dllimport) void EndScissorMode();

extern __declspec(dllimport) void BeginVrStereoMode(struct VrStereoConfig config);

extern __declspec(dllimport) void EndVrStereoMode();

extern __declspec(dllimport) struct VrStereoConfig LoadVrStereoConfig(struct VrDeviceInfo device);

extern __declspec(dllimport) void UnloadVrStereoConfig(struct VrStereoConfig config);

extern __declspec(dllimport) struct Shader LoadShader(i8* vsFileName,i8* fsFileName);

extern __declspec(dllimport) struct Shader LoadShaderFromMemory(i8* vsCode,i8* fsCode);

extern __declspec(dllimport) bool IsShaderReady(struct Shader shader);

extern __declspec(dllimport) i32 GetShaderLocation(struct Shader shader,i8* uniformName);

extern __declspec(dllimport) i32 GetShaderLocationAttrib(struct Shader shader,i8* attribName);

extern __declspec(dllimport) void SetShaderValue(struct Shader shader,i32 locIndex,u8* value,i32 uniformType);

extern __declspec(dllimport) void SetShaderValueV(struct Shader shader,i32 locIndex,void* value,i32 uniformType,i32 count);

extern __declspec(dllimport) void SetShaderValueMatrix(struct Shader shader,i32 locIndex,struct Matrix mat);

extern __declspec(dllimport) void SetShaderValueTexture(struct Shader shader,i32 locIndex,struct Texture texture);

extern __declspec(dllimport) void UnloadShader(struct Shader shader);

extern __declspec(dllimport) struct Ray GetMouseRay(struct Vector2 mousePosition,struct Camera3D camera);

extern __declspec(dllimport) struct Matrix GetCameraMatrix(struct Camera3D camera);

extern __declspec(dllimport) struct Matrix GetCameraMatrix2D(struct Camera2D camera);

extern __declspec(dllimport) struct Vector2 GetWorldToScreen(struct Vector3 position,struct Camera3D camera);

extern __declspec(dllimport) struct Vector2 GetScreenToWorld2D(struct Vector2 position,struct Camera2D camera);

extern __declspec(dllimport) struct Vector2 GetWorldToScreenEx(struct Vector3 position,struct Camera3D camera,i32 width,i32 height);

extern __declspec(dllimport) struct Vector2 GetWorldToScreen2D(struct Vector2 position,struct Camera2D camera);

extern __declspec(dllimport) void SetTargetFPS(i32 fps);

extern __declspec(dllimport) float GetFrameTime();

extern __declspec(dllimport) f64 GetTime();

extern __declspec(dllimport) i32 GetFPS();

extern __declspec(dllimport) void SwapScreenBuffer();

extern __declspec(dllimport) void PollInputEvents();

extern __declspec(dllimport) void WaitTime(f64 seconds);

extern __declspec(dllimport) void SetRandomSeed(u32 seed);

extern __declspec(dllimport) i32 GetRandomValue(i32 min,i32 max);

extern __declspec(dllimport) i32* LoadRandomSequence(u32 count,i32 min,i32 max);

extern __declspec(dllimport) void UnloadRandomSequence(i32* sequence);

extern __declspec(dllimport) void TakeScreenshot(i8* fileName);

extern __declspec(dllimport) void SetConfigFlags(u32 flags);

extern __declspec(dllimport) void OpenURL(i8* url);

extern __declspec(dllimport) void SetTraceLogLevel(i32 logLevel);

extern __declspec(dllimport) void* MemAlloc(u32 size);

extern __declspec(dllimport) void* MemRealloc(u8* ptr,u32 size);

extern __declspec(dllimport) void MemFree(void* ptr);

extern __declspec(dllimport) void SetTraceLogCallback(__Fv3i32pi8pi8 callback);

extern __declspec(dllimport) void SetLoadFileDataCallback(__Fpi82pi8pi32 callback);

extern __declspec(dllimport) void SetSaveFileDataCallback(__Fb3pi8pu8i32 callback);

extern __declspec(dllimport) void SetLoadFileTextCallback(__Fpi81pi8 callback);

extern __declspec(dllimport) void SetSaveFileTextCallback(__Fb2pi8pi8 callback);

extern __declspec(dllimport) u8* LoadFileData(i8* fileName,i32* dataSize);

extern __declspec(dllimport) void UnloadFileData(i8* data);

extern __declspec(dllimport) bool SaveFileData(i8* fileName,u8* data,i32 dataSize);

extern __declspec(dllimport) bool ExportDataAsCode(u8* data,i32 dataSize,i8* fileName);

extern __declspec(dllimport) i8* LoadFileText(i8* fileName);

extern __declspec(dllimport) void UnloadFileText(i8* text);

extern __declspec(dllimport) bool SaveFileText(i8* fileName,i8* text);

extern __declspec(dllimport) bool FileExists(i8* fileName);

extern __declspec(dllimport) bool DirectoryExists(i8* dirPath);

extern __declspec(dllimport) bool IsFileExtension(i8* fileName,i8* ext);

extern __declspec(dllimport) i32 GetFileLength(i8* fileName);

extern __declspec(dllimport) i8* GetFileExtension(i8* fileName);

extern __declspec(dllimport) i8* GetFileName(i8* filePath);

extern __declspec(dllimport) i8* GetFileNameWithoutExt(i8* filePath);

extern __declspec(dllimport) i8* GetDirectoryPath(i8* filePath);

extern __declspec(dllimport) i8* GetPrevDirectoryPath(i8* dirPath);

extern __declspec(dllimport) i8* GetWorkingDirectory();

extern __declspec(dllimport) i8* GetApplicationDirectory();

extern __declspec(dllimport) bool ChangeDirectory(i8* dir);

extern __declspec(dllimport) bool IsPathFile(i8* path);

extern __declspec(dllimport) struct FilePathList LoadDirectoryFiles(i8* dirPath);

extern __declspec(dllimport) struct FilePathList LoadDirectoryFilesEx(i8* basePath,i8* filter,bool scanSubdirs);

extern __declspec(dllimport) void UnloadDirectoryFiles(struct FilePathList files);

extern __declspec(dllimport) bool IsFileDropped();

extern __declspec(dllimport) struct FilePathList LoadDroppedFiles();

extern __declspec(dllimport) void UnloadDroppedFiles(struct FilePathList files);

extern __declspec(dllimport) i64 GetFileModTime(i8* fileName);

extern __declspec(dllimport) u8* CompressData(u8* data,i32 dataSize,i32* compDataSize);

extern __declspec(dllimport) u8* DecompressData(u8* compData,i32 compDataSize,i32* dataSize);

extern __declspec(dllimport) i8* EncodeDataBase64(u8* data,i32 dataSize,i32* outputSize);

extern __declspec(dllimport) u8* DecodeDataBase64(u8* data,i32* outputSize);

extern __declspec(dllimport) struct AutomationEventList LoadAutomationEventList(i8* fileName);

extern __declspec(dllimport) void UnloadAutomationEventList(struct AutomationEventList* list);

extern __declspec(dllimport) bool ExportAutomationEventList(struct AutomationEventList list,i8* fileName);

extern __declspec(dllimport) void SetAutomationEventList(struct AutomationEventList* list);

extern __declspec(dllimport) void SetAutomationEventBaseFrame(i32 frame);

extern __declspec(dllimport) void StartAutomationEventRecording();

extern __declspec(dllimport) void StopAutomationEventRecording();

extern __declspec(dllimport) void PlayAutomationEvent(struct AutomationEvent event);

extern __declspec(dllimport) bool IsKeyPressed(i32 key);

extern __declspec(dllimport) bool IsKeyPressedRepeat(i32 key);

extern __declspec(dllimport) bool IsKeyDown(i32 key);

extern __declspec(dllimport) bool IsKeyReleased(i32 key);

extern __declspec(dllimport) bool IsKeyUp(i32 key);

extern __declspec(dllimport) i32 GetKeyPressed();

extern __declspec(dllimport) i32 GetCharPressed();

extern __declspec(dllimport) void SetExitKey(i32 key);

extern __declspec(dllimport) bool IsGamepadAvailable(i32 gamepad);

extern __declspec(dllimport) i8* GetGamepadName(i32 gamepad);

extern __declspec(dllimport) bool IsGamepadButtonPressed(i32 gamepad,i32 button);

extern __declspec(dllimport) bool IsGamepadButtonDown(i32 gamepad,i32 button);

extern __declspec(dllimport) bool IsGamepadButtonReleased(i32 gamepad,i32 button);

extern __declspec(dllimport) bool IsGamepadButtonUp(i32 gamepad,i32 button);

extern __declspec(dllimport) i32 GetGamepadButtonPressed();

extern __declspec(dllimport) i32 GetGamepadAxisCount(i32 gamepad);

extern __declspec(dllimport) float GetGamepadAxisMovement(i32 gamepad,i32 axis);

extern __declspec(dllimport) i32 SetGamepadMappings(i8* mappings);

extern __declspec(dllimport) bool IsMouseButtonPressed(i32 button);

extern __declspec(dllimport) bool IsMouseButtonDown(i32 button);

extern __declspec(dllimport) bool IsMouseButtonReleased(i32 button);

extern __declspec(dllimport) bool IsMouseButtonUp(i32 button);

extern __declspec(dllimport) i32 GetMouseX();

extern __declspec(dllimport) i32 GetMouseY();

extern __declspec(dllimport) struct Vector2 GetMousePosition();

extern __declspec(dllimport) struct Vector2 GetMouseDelta();

extern __declspec(dllimport) void SetMousePosition(i32 x,i32 y);

extern __declspec(dllimport) void SetMouseOffset(i32 offsetX,i32 offsetY);

extern __declspec(dllimport) void SetMouseScale(float scaleX,float scaleY);

extern __declspec(dllimport) float GetMouseWheelMove();

extern __declspec(dllimport) struct Vector2 GetMouseWheelMoveV();

extern __declspec(dllimport) void SetMouseCursor(i32 cursor);

extern __declspec(dllimport) i32 GetTouchX();

extern __declspec(dllimport) i32 GetTouchY();

extern __declspec(dllimport) struct Vector2 GetTouchPosition(i32 index);

extern __declspec(dllimport) i32 GetTouchPointId(i32 index);

extern __declspec(dllimport) i32 GetTouchPointCount();

extern __declspec(dllimport) void SetGesturesEnabled(u32 flags);

extern __declspec(dllimport) bool IsGestureDetected(u32 gesture);

extern __declspec(dllimport) i32 GetGestureDetected();

extern __declspec(dllimport) float GetGestureHoldDuration();

extern __declspec(dllimport) struct Vector2 GetGestureDragVector();

extern __declspec(dllimport) float GetGestureDragAngle();

extern __declspec(dllimport) struct Vector2 GetGesturePinchVector();

extern __declspec(dllimport) float GetGesturePinchAngle();

extern __declspec(dllimport) void UpdateCamera(struct Camera3D* camera,i32 mode);

extern __declspec(dllimport) void UpdateCameraPro(struct Camera3D* camera,struct Vector3 movement,struct Vector3 rotation,float zoom);

extern __declspec(dllimport) void SetShapesTexture(struct Texture texture,struct Rectangle source);

extern __declspec(dllimport) void DrawPixel(i32 posX,i32 posY,struct Color color);

extern __declspec(dllimport) void DrawPixelV(struct Vector2 position,struct Color color);

extern __declspec(dllimport) void DrawLine(i32 startPosX,i32 startPosY,i32 endPosX,i32 endPosY,struct Color color);

extern __declspec(dllimport) void DrawLineV(struct Vector2 startPos,struct Vector2 endPos,struct Color color);

extern __declspec(dllimport) void DrawLineEx(struct Vector2 startPos,struct Vector2 endPos,float thick,struct Color color);

extern __declspec(dllimport) void DrawLineStrip(struct Vector2* points,i32 pointCount,struct Color color);

extern __declspec(dllimport) void DrawLineBezier(struct Vector2 startPos,struct Vector2 endPos,float thick,struct Color color);

extern __declspec(dllimport) void DrawCircle(i32 centerX,i32 centerY,float radius,struct Color color);

extern __declspec(dllimport) void DrawCircleSector(struct Vector2 center,float radius,float startAngle,float endAngle,i32 segments,struct Color color);

extern __declspec(dllimport) void DrawCircleSectorLines(struct Vector2 center,float radius,float startAngle,float endAngle,i32 segments,struct Color color);

extern __declspec(dllimport) void DrawCircleGradient(i32 centerX,i32 centerY,float radius,struct Color color1,struct Color color2);

extern __declspec(dllimport) void DrawCircleV(struct Vector2 center,float radius,struct Color color);

extern __declspec(dllimport) void DrawCircleLines(i32 centerX,i32 centerY,float radius,struct Color color);

extern __declspec(dllimport) void DrawCircleLinesV(struct Vector2 center,float radius,struct Color color);

extern __declspec(dllimport) void DrawEllipse(i32 centerX,i32 centerY,float radiusH,float radiusV,struct Color color);

extern __declspec(dllimport) void DrawEllipseLines(i32 centerX,i32 centerY,float radiusH,float radiusV,struct Color color);

extern __declspec(dllimport) void DrawRing(struct Vector2 center,float innerRadius,float outerRadius,float startAngle,float endAngle,i32 segments,struct Color color);

extern __declspec(dllimport) void DrawRingLines(struct Vector2 center,float innerRadius,float outerRadius,float startAngle,float endAngle,i32 segments,struct Color color);

extern __declspec(dllimport) void DrawRectangle(i32 posX,i32 posY,i32 width,i32 height,struct Color color);

extern __declspec(dllimport) void DrawRectangleV(struct Vector2 position,struct Vector2 size,struct Color color);

extern __declspec(dllimport) void DrawRectangleRec(struct Rectangle rec,struct Color color);

extern __declspec(dllimport) void DrawRectanglePro(struct Rectangle rec,struct Vector2 origin,float rotation,struct Color color);

extern __declspec(dllimport) void DrawRectangleGradientV(i32 posX,i32 posY,i32 width,i32 height,struct Color color1,struct Color color2);

extern __declspec(dllimport) void DrawRectangleGradientH(i32 posX,i32 posY,i32 width,i32 height,struct Color color1,struct Color color2);

extern __declspec(dllimport) void DrawRectangleGradientEx(struct Rectangle rec,struct Color col1,struct Color col2,struct Color col3,struct Color col4);

extern __declspec(dllimport) void DrawRectangleLines(i32 posX,i32 posY,i32 width,i32 height,struct Color color);

extern __declspec(dllimport) void DrawRectangleLinesEx(struct Rectangle rec,float lineThick,struct Color color);

extern __declspec(dllimport) void DrawRectangleRounded(struct Rectangle rec,float roundness,i32 segments,struct Color color);

extern __declspec(dllimport) void DrawRectangleRoundedLines(struct Rectangle rec,float roundness,i32 segments,float lineThick,struct Color color);

extern __declspec(dllimport) void DrawTriangle(struct Vector2 v1,struct Vector2 v2,struct Vector2 v3,struct Color color);

extern __declspec(dllimport) void DrawTriangleLines(struct Vector2 v1,struct Vector2 v2,struct Vector2 v3,struct Color color);

extern __declspec(dllimport) void DrawTriangleFan(struct Vector2* points,i32 pointCount,struct Color color);

extern __declspec(dllimport) void DrawTriangleStrip(struct Vector2* points,i32 pointCount,struct Color color);

extern __declspec(dllimport) void DrawPoly(struct Vector2 center,i32 sides,float radius,float rotation,struct Color color);

extern __declspec(dllimport) void DrawPolyLines(struct Vector2 center,i32 sides,float radius,float rotation,struct Color color);

extern __declspec(dllimport) void DrawPolyLinesEx(struct Vector2 center,i32 sides,float radius,float rotation,float lineThick,struct Color color);

extern __declspec(dllimport) void DrawSplineLinear(struct Vector2* points,i32 pointCount,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineBasis(struct Vector2* points,i32 pointCount,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineCatmullRom(struct Vector2* points,i32 pointCount,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineBezierQuadratic(struct Vector2* points,i32 pointCount,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineBezierCubic(struct Vector2* points,i32 pointCount,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineSegmentLinear(struct Vector2 p1,struct Vector2 p2,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineSegmentBasis(struct Vector2 p1,struct Vector2 p2,struct Vector2 p3,struct Vector2 p4,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineSegmentCatmullRom(struct Vector2 p1,struct Vector2 p2,struct Vector2 p3,struct Vector2 p4,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineSegmentBezierQuadratic(struct Vector2 p1,struct Vector2 c2,struct Vector2 p3,float thick,struct Color color);

extern __declspec(dllimport) void DrawSplineSegmentBezierCubic(struct Vector2 p1,struct Vector2 c2,struct Vector2 c3,struct Vector2 p4,float thick,struct Color color);

extern __declspec(dllimport) struct Vector2 GetSplinePointLinear(struct Vector2 startPos,struct Vector2 endPos,float t);

extern __declspec(dllimport) struct Vector2 GetSplinePointBasis(struct Vector2 p1,struct Vector2 p2,struct Vector2 p3,struct Vector2 p4,float t);

extern __declspec(dllimport) struct Vector2 GetSplinePointCatmullRom(struct Vector2 p1,struct Vector2 p2,struct Vector2 p3,struct Vector2 p4,float t);

extern __declspec(dllimport) struct Vector2 GetSplinePointBezierQuad(struct Vector2 p1,struct Vector2 c2,struct Vector2 p3,float t);

extern __declspec(dllimport) struct Vector2 GetSplinePointBezierCubic(struct Vector2 p1,struct Vector2 c2,struct Vector2 c3,struct Vector2 p4,float t);

extern __declspec(dllimport) bool CheckCollisionRecs(struct Rectangle rec1,struct Rectangle rec2);

extern __declspec(dllimport) bool CheckCollisionCircles(struct Vector2 center1,float radius1,struct Vector2 center2,float radius2);

extern __declspec(dllimport) bool CheckCollisionCircleRec(struct Vector2 center,float radius,struct Rectangle rec);

extern __declspec(dllimport) bool CheckCollisionPointRec(struct Vector2 point,struct Rectangle rec);

extern __declspec(dllimport) bool CheckCollisionPointCircle(struct Vector2 point,struct Vector2 center,float radius);

extern __declspec(dllimport) bool CheckCollisionPointTriangle(struct Vector2 point,struct Vector2 p1,struct Vector2 p2,struct Vector2 p3);

extern __declspec(dllimport) bool CheckCollisionPointPoly(struct Vector2 point,struct Vector2* points,i32 pointCount);

extern __declspec(dllimport) bool CheckCollisionLines(struct Vector2 startPos1,struct Vector2 endPos1,struct Vector2 startPos2,struct Vector2 endPos2,struct Vector2* collisionPoint);

extern __declspec(dllimport) bool CheckCollisionPointLine(struct Vector2 point,struct Vector2 p1,struct Vector2 p2,i32 threshold);

extern __declspec(dllimport) struct Rectangle GetCollisionRec(struct Rectangle rec1,struct Rectangle rec2);

extern __declspec(dllimport) struct Image LoadImage(i8* fileName);

extern __declspec(dllimport) struct Image LoadImageRaw(i8* fileName,i32 width,i32 height,i32 format,i32 headerSize);

extern __declspec(dllimport) struct Image LoadImageSvg(i8* fileNameOrString,i32 width,i32 height);

extern __declspec(dllimport) struct Image LoadImageAnim(i8* fileName,i32* frames);

extern __declspec(dllimport) struct Image LoadImageFromMemory(i8* fileType,u8* fileData,i32 dataSize);

extern __declspec(dllimport) struct Image LoadImageFromTexture(struct Texture texture);

extern __declspec(dllimport) struct Image LoadImageFromScreen();

extern __declspec(dllimport) bool IsImageReady(struct Image image);

extern __declspec(dllimport) void UnloadImage(struct Image image);

extern __declspec(dllimport) bool ExportImage(struct Image image,i8* fileName);

extern __declspec(dllimport) u8* ExportImageToMemory(struct Image image,i8* fileType,i32* fileSize);

extern __declspec(dllimport) bool ExportImageAsCode(struct Image image,i8* fileName);

extern __declspec(dllimport) struct Image GenImageColor(i32 width,i32 height,struct Color color);

extern __declspec(dllimport) struct Image GenImageGradientLinear(i32 width,i32 height,i32 direction,struct Color start,struct Color end);

extern __declspec(dllimport) struct Image GenImageGradientRadial(i32 width,i32 height,float density,struct Color inner,struct Color outer);

extern __declspec(dllimport) struct Image GenImageGradientSquare(i32 width,i32 height,float density,struct Color inner,struct Color outer);

extern __declspec(dllimport) struct Image GenImageChecked(i32 width,i32 height,i32 checksX,i32 checksY,struct Color col1,struct Color col2);

extern __declspec(dllimport) struct Image GenImageWhiteNoise(i32 width,i32 height,float factor);

extern __declspec(dllimport) struct Image GenImagePerlinNoise(i32 width,i32 height,i32 offsetX,i32 offsetY,float scale);

extern __declspec(dllimport) struct Image GenImageCellular(i32 width,i32 height,i32 tileSize);

extern __declspec(dllimport) struct Image GenImageText(i32 width,i32 height,i8* text);

extern __declspec(dllimport) struct Image ImageCopy(struct Image image);

extern __declspec(dllimport) struct Image ImageFromImage(struct Image image,struct Rectangle rec);

extern __declspec(dllimport) struct Image ImageText(i8* text,i32 fontSize,struct Color color);

extern __declspec(dllimport) struct Image ImageTextEx(struct Font font,i8* text,float fontSize,float spacing,struct Color tint);

extern __declspec(dllimport) void ImageFormat(struct Image* image,i32 newFormat);

extern __declspec(dllimport) void ImageToPOT(struct Image* image,struct Color fill);

extern __declspec(dllimport) void ImageCrop(struct Image* image,struct Rectangle crop);

extern __declspec(dllimport) void ImageAlphaCrop(struct Image* image,float threshold);

extern __declspec(dllimport) void ImageAlphaClear(struct Image* image,struct Color color,float threshold);

extern __declspec(dllimport) void ImageAlphaMask(struct Image* image,struct Image alphaMask);

extern __declspec(dllimport) void ImageAlphaPremultiply(struct Image* image);

extern __declspec(dllimport) void ImageBlurGaussian(struct Image* image,i32 blurSize);

extern __declspec(dllimport) void ImageResize(struct Image* image,i32 newWidth,i32 newHeight);

extern __declspec(dllimport) void ImageResizeNN(struct Image* image,i32 newWidth,i32 newHeight);

extern __declspec(dllimport) void ImageResizeCanvas(struct Image* image,i32 newWidth,i32 newHeight,i32 offsetX,i32 offsetY,struct Color fill);

extern __declspec(dllimport) void ImageMipmaps(struct Image* image);

extern __declspec(dllimport) void ImageDither(struct Image* image,i32 rBpp,i32 gBpp,i32 bBpp,i32 aBpp);

extern __declspec(dllimport) void ImageFlipVertical(struct Image* image);

extern __declspec(dllimport) void ImageFlipHorizontal(struct Image* image);

extern __declspec(dllimport) void ImageRotate(struct Image* image,i32 degrees);

extern __declspec(dllimport) void ImageRotateCW(struct Image* image);

extern __declspec(dllimport) void ImageRotateCCW(struct Image* image);

extern __declspec(dllimport) void ImageColorTint(struct Image* image,struct Color color);

extern __declspec(dllimport) void ImageColorInvert(struct Image* image);

extern __declspec(dllimport) void ImageColorGrayscale(struct Image* image);

extern __declspec(dllimport) void ImageColorContrast(struct Image* image,float contrast);

extern __declspec(dllimport) void ImageColorBrightness(struct Image* image,i32 brightness);

extern __declspec(dllimport) void ImageColorReplace(struct Image* image,struct Color color,struct Color replace);

extern __declspec(dllimport) struct Color* LoadImageColors(struct Image image);

extern __declspec(dllimport) struct Color* LoadImagePalette(struct Image image,i32 maxPaletteSize,i32* colorCount);

extern __declspec(dllimport) void UnloadImageColors(struct Color* colors);

extern __declspec(dllimport) void UnloadImagePalette(struct Color* colors);

extern __declspec(dllimport) struct Rectangle GetImageAlphaBorder(struct Image image,float threshold);

extern __declspec(dllimport) struct Color GetImageColor(struct Image image,i32 x,i32 y);

extern __declspec(dllimport) void ImageClearBackground(struct Image* dst,struct Color color);

extern __declspec(dllimport) void ImageDrawPixel(struct Image* dst,i32 posX,i32 posY,struct Color color);

extern __declspec(dllimport) void ImageDrawPixelV(struct Image* dst,struct Vector2 position,struct Color color);

extern __declspec(dllimport) void ImageDrawLine(struct Image* dst,i32 startPosX,i32 startPosY,i32 endPosX,i32 endPosY,struct Color color);

extern __declspec(dllimport) void ImageDrawLineV(struct Image* dst,struct Vector2 start,struct Vector2 end,struct Color color);

extern __declspec(dllimport) void ImageDrawCircle(struct Image* dst,i32 centerX,i32 centerY,i32 radius,struct Color color);

extern __declspec(dllimport) void ImageDrawCircleV(struct Image* dst,struct Vector2 center,i32 radius,struct Color color);

extern __declspec(dllimport) void ImageDrawCircleLines(struct Image* dst,i32 centerX,i32 centerY,i32 radius,struct Color color);

extern __declspec(dllimport) void ImageDrawCircleLinesV(struct Image* dst,struct Vector2 center,i32 radius,struct Color color);

extern __declspec(dllimport) void ImageDrawRectangle(struct Image* dst,i32 posX,i32 posY,i32 width,i32 height,struct Color color);

extern __declspec(dllimport) void ImageDrawRectangleV(struct Image* dst,struct Vector2 position,struct Vector2 size,struct Color color);

extern __declspec(dllimport) void ImageDrawRectangleRec(struct Image* dst,struct Rectangle rec,struct Color color);

extern __declspec(dllimport) void ImageDrawRectangleLines(struct Image* dst,struct Rectangle rec,i32 thick,struct Color color);

extern __declspec(dllimport) void ImageDraw(struct Image* dst,struct Image src,struct Rectangle srcRec,struct Rectangle dstRec,struct Color tint);

extern __declspec(dllimport) void ImageDrawText(struct Image* dst,i8* text,i32 posX,i32 posY,i32 fontSize,struct Color color);

extern __declspec(dllimport) void ImageDrawTextEx(struct Image* dst,struct Font font,i8* text,struct Vector2 position,float fontSize,float spacing,struct Color tint);

extern __declspec(dllimport) struct Texture LoadTexture(i8* fileName);

extern __declspec(dllimport) struct Texture LoadTextureFromImage(struct Image image);

extern __declspec(dllimport) struct Texture LoadTextureCubemap(struct Image image,i32 layout);

extern __declspec(dllimport) struct RenderTexture LoadRenderTexture(i32 width,i32 height);

extern __declspec(dllimport) bool IsTextureReady(struct Texture texture);

extern __declspec(dllimport) void UnloadTexture(struct Texture texture);

extern __declspec(dllimport) bool IsRenderTextureReady(struct RenderTexture target);

extern __declspec(dllimport) void UnloadRenderTexture(struct RenderTexture target);

extern __declspec(dllimport) void UpdateTexture(struct Texture texture,u8* pixels);

extern __declspec(dllimport) void UpdateTextureRec(struct Texture texture,struct Rectangle rec,u8* pixels);

extern __declspec(dllimport) void GenTextureMipmaps(struct Texture* texture);

extern __declspec(dllimport) void SetTextureFilter(struct Texture texture,i32 filter);

extern __declspec(dllimport) void SetTextureWrap(struct Texture texture,i32 wrap);

extern __declspec(dllimport) void DrawTexture(struct Texture texture,i32 posX,i32 posY,struct Color tint);

extern __declspec(dllimport) void DrawTextureV(struct Texture texture,struct Vector2 position,struct Color tint);

extern __declspec(dllimport) void DrawTextureEx(struct Texture texture,struct Vector2 position,float rotation,float scale,struct Color tint);

extern __declspec(dllimport) void DrawTextureRec(struct Texture texture,struct Rectangle source,struct Vector2 position,struct Color tint);

extern __declspec(dllimport) void DrawTexturePro(struct Texture texture,struct Rectangle source,struct Rectangle dest,struct Vector2 origin,float rotation,struct Color tint);

extern __declspec(dllimport) void DrawTextureNPatch(struct Texture texture,struct NPatchInfo nPatchInfo,struct Rectangle dest,struct Vector2 origin,float rotation,struct Color tint);

extern __declspec(dllimport) struct Color Fade(struct Color color,float alpha);

extern __declspec(dllimport) i32 ColorToInt(struct Color color);

extern __declspec(dllimport) struct Vector4 ColorNormalize(struct Color color);

extern __declspec(dllimport) struct Color ColorFromNormalized(struct Vector4 normalized);

extern __declspec(dllimport) struct Vector3 ColorToHSV(struct Color color);

extern __declspec(dllimport) struct Color ColorFromHSV(float hue,float saturation,float value);

extern __declspec(dllimport) struct Color ColorTint(struct Color color,struct Color tint);

extern __declspec(dllimport) struct Color ColorBrightness(struct Color color,float factor);

extern __declspec(dllimport) struct Color ColorContrast(struct Color color,float contrast);

extern __declspec(dllimport) struct Color ColorAlpha(struct Color color,float alpha);

extern __declspec(dllimport) struct Color ColorAlphaBlend(struct Color dst,struct Color src,struct Color tint);

extern __declspec(dllimport) struct Color GetColor(u32 hexValue);

extern __declspec(dllimport) struct Color GetPixelColor(u8* srcPtr,i32 format);

extern __declspec(dllimport) void SetPixelColor(u8* dstPtr,struct Color color,i32 format);

extern __declspec(dllimport) i32 GetPixelDataSize(i32 width,i32 height,i32 format);

extern __declspec(dllimport) struct Font GetFontDefault();

extern __declspec(dllimport) struct Font LoadFont(i8* fileName);

extern __declspec(dllimport) struct Font LoadFontEx(i8* fileName,i32 fontSize,i32* codepoints,i32 codepointCount);

extern __declspec(dllimport) struct Font LoadFontFromImage(struct Image image,struct Color key,i32 firstChar);

extern __declspec(dllimport) struct Font LoadFontFromMemory(i8* fileType,u8* fileData,i32 dataSize,i32 fontSize,i32* codepoints,i32 codepointCount);

extern __declspec(dllimport) bool IsFontReady(struct Font font);

extern __declspec(dllimport) struct GlyphInfo* LoadFontData(u8* fileData,i32 dataSize,i32 fontSize,i32* codepoints,i32 codepointCount,i32 type);

extern __declspec(dllimport) struct Image GenImageFontAtlas(struct GlyphInfo* glyphs,struct Rectangle** glyphRecs,i32 glyphCount,i32 fontSize,i32 padding,i32 packMethod);

extern __declspec(dllimport) void UnloadFontData(struct GlyphInfo* glyphs,i32 glyphCount);

extern __declspec(dllimport) void UnloadFont(struct Font font);

extern __declspec(dllimport) bool ExportFontAsCode(struct Font font,i8* fileName);

extern __declspec(dllimport) void DrawFPS(i32 posX,i32 posY);

extern __declspec(dllimport) void DrawText(i8* text,i32 posX,i32 posY,i32 fontSize,struct Color color);

extern __declspec(dllimport) void DrawTextEx(struct Font font,i8* text,struct Vector2 position,float fontSize,float spacing,struct Color tint);

extern __declspec(dllimport) void DrawTextPro(struct Font font,i8* text,struct Vector2 position,struct Vector2 origin,float rotation,float fontSize,float spacing,struct Color tint);

extern __declspec(dllimport) void DrawTextCodepoint(struct Font font,i32 codepoint,struct Vector2 position,float fontSize,struct Color tint);

extern __declspec(dllimport) void DrawTextCodepoints(struct Font font,i32* codepoints,i32 codepointCount,struct Vector2 position,float fontSize,float spacing,struct Color tint);

extern __declspec(dllimport) void SetTextLineSpacing(i32 spacing);

extern __declspec(dllimport) i32 MeasureText(i8* text,i32 fontSize);

extern __declspec(dllimport) struct Vector2 MeasureTextEx(struct Font font,i8* text,float fontSize,float spacing);

extern __declspec(dllimport) i32 GetGlyphIndex(struct Font font,i32 codepoint);

extern __declspec(dllimport) struct GlyphInfo GetGlyphInfo(struct Font font,i32 codepoint);

extern __declspec(dllimport) struct Rectangle GetGlyphAtlasRec(struct Font font,i32 codepoint);

extern __declspec(dllimport) i8* LoadUTF8(i32* codepoints,i32 length);

extern __declspec(dllimport) void UnloadUTF8(i8* text);

extern __declspec(dllimport) i32* LoadCodepoints(i8* text,i32* count);

extern __declspec(dllimport) void UnloadCodepoints(i32* codepoints);

extern __declspec(dllimport) i32 GetCodepointCount(i8* text);

extern __declspec(dllimport) i32 GetCodepoint(i8* text,i32* codepointSize);

extern __declspec(dllimport) i32 GetCodepointNext(i8* text,i32* codepointSize);

extern __declspec(dllimport) i32 GetCodepointPrevious(i8* text,i32* codepointSize);

extern __declspec(dllimport) i8* CodepointToUTF8(i32 codepoint,i32* utf8Size);

extern __declspec(dllimport) i32 TextCopy(i8* dst,i8* src);

extern __declspec(dllimport) bool TextIsEqual(i8* text1,i8* text2);

extern __declspec(dllimport) u32 TextLength(i8* text);

extern __declspec(dllimport) i8* TextSubtext(i8* text,i32 position,i32 length);

extern __declspec(dllimport) i8* TextReplace(i8* text,i8* replace,i8* by);

extern __declspec(dllimport) i8* TextInsert(i8* text,i8* insert,i32 position);

extern __declspec(dllimport) i8* TextJoin(i8** textList,i32 count,i8* delimiter);

extern __declspec(dllimport) i8** TextSplit(i8* text,i8 delimiter,i32* count);

extern __declspec(dllimport) void TextAppend(i8* text,i8* append,i32* position);

extern __declspec(dllimport) i32 TextFindIndex(i8* text,i8* find);

extern __declspec(dllimport) i8* TextToUpper(i8* text);

extern __declspec(dllimport) i8* TextToLower(i8* text);

extern __declspec(dllimport) i8* TextToPascal(i8* text);

extern __declspec(dllimport) i32 TextToInteger(i8* text);

extern __declspec(dllimport) void DrawLine3D(struct Vector3 startPos,struct Vector3 endPos,struct Color color);

extern __declspec(dllimport) void DrawPoint3D(struct Vector3 position,struct Color color);

extern __declspec(dllimport) void DrawCircle3D(struct Vector3 center,float radius,struct Vector3 rotationAxis,float rotationAngle,struct Color color);

extern __declspec(dllimport) void DrawTriangle3D(struct Vector3 v1,struct Vector3 v2,struct Vector3 v3,struct Color color);

extern __declspec(dllimport) void DrawTriangleStrip3D(struct Vector3* points,i32 pointCount,struct Color color);

extern __declspec(dllimport) void DrawCube(struct Vector3 position,float width,float height,float length,struct Color color);

extern __declspec(dllimport) void DrawCubeV(struct Vector3 position,struct Vector3 size,struct Color color);

extern __declspec(dllimport) void DrawCubeWires(struct Vector3 position,float width,float height,float length,struct Color color);

extern __declspec(dllimport) void DrawCubeWiresV(struct Vector3 position,struct Vector3 size,struct Color color);

extern __declspec(dllimport) void DrawSphere(struct Vector3 centerPos,float radius,struct Color color);

extern __declspec(dllimport) void DrawSphereEx(struct Vector3 centerPos,float radius,i32 rings,i32 slices,struct Color color);

extern __declspec(dllimport) void DrawSphereWires(struct Vector3 centerPos,float radius,i32 rings,i32 slices,struct Color color);

extern __declspec(dllimport) void DrawCylinder(struct Vector3 position,float radiusTop,float radiusBottom,float height,i32 slices,struct Color color);

extern __declspec(dllimport) void DrawCylinderEx(struct Vector3 startPos,struct Vector3 endPos,float startRadius,float endRadius,i32 sides,struct Color color);

extern __declspec(dllimport) void DrawCylinderWires(struct Vector3 position,float radiusTop,float radiusBottom,float height,i32 slices,struct Color color);

extern __declspec(dllimport) void DrawCylinderWiresEx(struct Vector3 startPos,struct Vector3 endPos,float startRadius,float endRadius,i32 sides,struct Color color);

extern __declspec(dllimport) void DrawCapsule(struct Vector3 startPos,struct Vector3 endPos,float radius,i32 slices,i32 rings,struct Color color);

extern __declspec(dllimport) void DrawCapsuleWires(struct Vector3 startPos,struct Vector3 endPos,float radius,i32 slices,i32 rings,struct Color color);

extern __declspec(dllimport) void DrawPlane(struct Vector3 centerPos,struct Vector2 size,struct Color color);

extern __declspec(dllimport) void DrawRay(struct Ray ray,struct Color color);

extern __declspec(dllimport) void DrawGrid(i32 slices,float spacing);

extern __declspec(dllimport) struct Model LoadModel(i8* fileName);

extern __declspec(dllimport) struct Model LoadModelFromMesh(struct Mesh mesh);

extern __declspec(dllimport) bool IsModelReady(struct Model model);

extern __declspec(dllimport) void UnloadModel(struct Model model);

extern __declspec(dllimport) struct BoundingBox GetModelBoundingBox(struct Model model);

extern __declspec(dllimport) void DrawModel(struct Model model,struct Vector3 position,float scale,struct Color tint);

extern __declspec(dllimport) void DrawModelEx(struct Model model,struct Vector3 position,struct Vector3 rotationAxis,float rotationAngle,struct Vector3 scale,struct Color tint);

extern __declspec(dllimport) void DrawModelWires(struct Model model,struct Vector3 position,float scale,struct Color tint);

extern __declspec(dllimport) void DrawModelWiresEx(struct Model model,struct Vector3 position,struct Vector3 rotationAxis,float rotationAngle,struct Vector3 scale,struct Color tint);

extern __declspec(dllimport) void DrawBoundingBox(struct BoundingBox box,struct Color color);

extern __declspec(dllimport) void DrawBillboard(struct Camera3D camera,struct Texture texture,struct Vector3 position,float size,struct Color tint);

extern __declspec(dllimport) void DrawBillboardRec(struct Camera3D camera,struct Texture texture,struct Rectangle source,struct Vector3 position,struct Vector2 size,struct Color tint);

extern __declspec(dllimport) void DrawBillboardPro(struct Camera3D camera,struct Texture texture,struct Rectangle source,struct Vector3 position,struct Vector3 up,struct Vector2 size,struct Vector2 origin,float rotation,struct Color tint);

extern __declspec(dllimport) void UploadMesh(struct Mesh* mesh,bool dynamic);

extern __declspec(dllimport) void UpdateMeshBuffer(struct Mesh mesh,i32 index,u8* data,i32 dataSize,i32 offset);

extern __declspec(dllimport) void UnloadMesh(struct Mesh mesh);

extern __declspec(dllimport) void DrawMesh(struct Mesh mesh,struct Material material,struct Matrix transform);

extern __declspec(dllimport) void DrawMeshInstanced(struct Mesh mesh,struct Material material,struct Matrix* transforms,i32 instances);

extern __declspec(dllimport) bool ExportMesh(struct Mesh mesh,i8* fileName);

extern __declspec(dllimport) struct BoundingBox GetMeshBoundingBox(struct Mesh mesh);

extern __declspec(dllimport) void GenMeshTangents(struct Mesh* mesh);

extern __declspec(dllimport) struct Mesh GenMeshPoly(i32 sides,float radius);

extern __declspec(dllimport) struct Mesh GenMeshPlane(float width,float length,i32 resX,i32 resZ);

extern __declspec(dllimport) struct Mesh GenMeshCube(float width,float height,float length);

extern __declspec(dllimport) struct Mesh GenMeshSphere(float radius,i32 rings,i32 slices);

extern __declspec(dllimport) struct Mesh GenMeshHemiSphere(float radius,i32 rings,i32 slices);

extern __declspec(dllimport) struct Mesh GenMeshCylinder(float radius,float height,i32 slices);

extern __declspec(dllimport) struct Mesh GenMeshCone(float radius,float height,i32 slices);

extern __declspec(dllimport) struct Mesh GenMeshTorus(float radius,float size,i32 radSeg,i32 sides);

extern __declspec(dllimport) struct Mesh GenMeshKnot(float radius,float size,i32 radSeg,i32 sides);

extern __declspec(dllimport) struct Mesh GenMeshHeightmap(struct Image heightmap,struct Vector3 size);

extern __declspec(dllimport) struct Mesh GenMeshCubicmap(struct Image cubicmap,struct Vector3 cubeSize);

extern __declspec(dllimport) struct Material* LoadMaterials(i8* fileName,i32* materialCount);

extern __declspec(dllimport) struct Material LoadMaterialDefault();

extern __declspec(dllimport) bool IsMaterialReady(struct Material material);

extern __declspec(dllimport) void UnloadMaterial(struct Material material);

extern __declspec(dllimport) void SetMaterialTexture(struct Material* material,i32 mapType,struct Texture texture);

extern __declspec(dllimport) void SetModelMeshMaterial(struct Model* model,i32 meshId,i32 materialId);

extern __declspec(dllimport) struct ModelAnimation* LoadModelAnimations(i8* fileName,i32* animCount);

extern __declspec(dllimport) void UpdateModelAnimation(struct Model model,struct ModelAnimation anim,i32 frame);

extern __declspec(dllimport) void UnloadModelAnimation(struct ModelAnimation anim);

extern __declspec(dllimport) void UnloadModelAnimations(struct ModelAnimation* animations,i32 animCount);

extern __declspec(dllimport) bool IsModelAnimationValid(struct Model model,struct ModelAnimation anim);

extern __declspec(dllimport) bool CheckCollisionSpheres(struct Vector3 center1,float radius1,struct Vector3 center2,float radius2);

extern __declspec(dllimport) bool CheckCollisionBoxes(struct BoundingBox box1,struct BoundingBox box2);

extern __declspec(dllimport) bool CheckCollisionBoxSphere(struct BoundingBox box,struct Vector3 center,float radius);

extern __declspec(dllimport) struct RayCollision GetRayCollisionSphere(struct Ray ray,struct Vector3 center,float radius);

extern __declspec(dllimport) struct RayCollision GetRayCollisionBox(struct Ray ray,struct BoundingBox box);

extern __declspec(dllimport) struct RayCollision GetRayCollisionMesh(struct Ray ray,struct Mesh mesh,struct Matrix transform);

extern __declspec(dllimport) struct RayCollision GetRayCollisionTriangle(struct Ray ray,struct Vector3 p1,struct Vector3 p2,struct Vector3 p3);

extern __declspec(dllimport) struct RayCollision GetRayCollisionQuad(struct Ray ray,struct Vector3 p1,struct Vector3 p2,struct Vector3 p3,struct Vector3 p4);

extern __declspec(dllimport) void InitAudioDevice();

extern __declspec(dllimport) void CloseAudioDevice();

extern __declspec(dllimport) bool IsAudioDeviceReady();

extern __declspec(dllimport) void SetMasterVolume(float volume);

extern __declspec(dllimport) float GetMasterVolume();

extern __declspec(dllimport) struct Wave LoadWave(i8* fileName);

extern __declspec(dllimport) struct Wave LoadWaveFromMemory(i8* fileType,u8* fileData,i32 dataSize);

extern __declspec(dllimport) bool IsWaveReady(struct Wave wave);

extern __declspec(dllimport) struct Sound LoadSound(i8* fileName);

extern __declspec(dllimport) struct Sound LoadSoundFromWave(struct Wave wave);

extern __declspec(dllimport) struct Sound LoadSoundAlias(struct Sound source);

extern __declspec(dllimport) bool IsSoundReady(struct Sound sound);

extern __declspec(dllimport) void UpdateSound(struct Sound sound,u8* data,i32 sampleCount);

extern __declspec(dllimport) void UnloadWave(struct Wave wave);

extern __declspec(dllimport) void UnloadSound(struct Sound sound);

extern __declspec(dllimport) void UnloadSoundAlias(struct Sound alias);

extern __declspec(dllimport) bool ExportWave(struct Wave wave,i8* fileName);

extern __declspec(dllimport) bool ExportWaveAsCode(struct Wave wave,i8* fileName);

extern __declspec(dllimport) void PlaySound(struct Sound sound);

extern __declspec(dllimport) void StopSound(struct Sound sound);

extern __declspec(dllimport) void PauseSound(struct Sound sound);

extern __declspec(dllimport) void ResumeSound(struct Sound sound);

extern __declspec(dllimport) bool IsSoundPlaying(struct Sound sound);

extern __declspec(dllimport) void SetSoundVolume(struct Sound sound,float volume);

extern __declspec(dllimport) void SetSoundPitch(struct Sound sound,float pitch);

extern __declspec(dllimport) void SetSoundPan(struct Sound sound,float pan);

extern __declspec(dllimport) struct Wave WaveCopy(struct Wave wave);

extern __declspec(dllimport) void WaveCrop(struct Wave* wave,i32 initSample,i32 finalSample);

extern __declspec(dllimport) void WaveFormat(struct Wave* wave,i32 sampleRate,i32 sampleSize,i32 channels);

extern __declspec(dllimport) float* LoadWaveSamples(struct Wave wave);

extern __declspec(dllimport) void UnloadWaveSamples(float* samples);

extern __declspec(dllimport) struct Music LoadMusicStream(i8* fileName);

extern __declspec(dllimport) struct Music LoadMusicStreamFromMemory(i8* fileType,u8* data,i32 dataSize);

extern __declspec(dllimport) bool IsMusicReady(struct Music music);

extern __declspec(dllimport) void UnloadMusicStream(struct Music music);

extern __declspec(dllimport) void PlayMusicStream(struct Music music);

extern __declspec(dllimport) bool IsMusicStreamPlaying(struct Music music);

extern __declspec(dllimport) void UpdateMusicStream(struct Music music);

extern __declspec(dllimport) void StopMusicStream(struct Music music);

extern __declspec(dllimport) void PauseMusicStream(struct Music music);

extern __declspec(dllimport) void ResumeMusicStream(struct Music music);

extern __declspec(dllimport) void SeekMusicStream(struct Music music,float position);

extern __declspec(dllimport) void SetMusicVolume(struct Music music,float volume);

extern __declspec(dllimport) void SetMusicPitch(struct Music music,float pitch);

extern __declspec(dllimport) void SetMusicPan(struct Music music,float pan);

extern __declspec(dllimport) float GetMusicTimeLength(struct Music music);

extern __declspec(dllimport) float GetMusicTimePlayed(struct Music music);

extern __declspec(dllimport) struct AudioStream LoadAudioStream(u32 sampleRate,u32 sampleSize,u32 channels);

extern __declspec(dllimport) bool IsAudioStreamReady(struct AudioStream stream);

extern __declspec(dllimport) void UnloadAudioStream(struct AudioStream stream);

extern __declspec(dllimport) void UpdateAudioStream(struct AudioStream stream,u8* data,i32 frameCount);

extern __declspec(dllimport) bool IsAudioStreamProcessed(struct AudioStream stream);

extern __declspec(dllimport) void PlayAudioStream(struct AudioStream stream);

extern __declspec(dllimport) void PauseAudioStream(struct AudioStream stream);

extern __declspec(dllimport) void ResumeAudioStream(struct AudioStream stream);

extern __declspec(dllimport) bool IsAudioStreamPlaying(struct AudioStream stream);

extern __declspec(dllimport) void StopAudioStream(struct AudioStream stream);

extern __declspec(dllimport) void SetAudioStreamVolume(struct AudioStream stream,float volume);

extern __declspec(dllimport) void SetAudioStreamPitch(struct AudioStream stream,float pitch);

extern __declspec(dllimport) void SetAudioStreamPan(struct AudioStream stream,float pan);

extern __declspec(dllimport) void SetAudioStreamBufferSizeDefault(i32 size);

extern __declspec(dllimport) void SetAudioStreamCallback(struct AudioStream stream,__Fv2pu8u32 callback);

extern __declspec(dllimport) void AttachAudioStreamProcessor(struct AudioStream stream,__Fv2pu8u32 processor);

extern __declspec(dllimport) void DetachAudioStreamProcessor(struct AudioStream stream,__Fv2pu8u32 processor);

extern __declspec(dllimport) void AttachAudioMixedProcessor(__Fv2pu8u32 processor);

extern __declspec(dllimport) void DetachAudioMixedProcessor(__Fv2pu8u32 processor);