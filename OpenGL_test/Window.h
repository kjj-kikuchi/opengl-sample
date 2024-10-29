#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// ウィンドウ関連の処理
class Window {
    // ウィンドウのハンドル
    GLFWwindow *const window;

    // 縦横比
    GLfloat size[2];

    // ワールド座標系に対するデバイス座標系の拡大率
    GLfloat scale;

    // 図形の正規化デバイス座標系上での位置
    GLfloat location[2];

    // キーボードの状態
    int keyStatus;

public:

    // コンストラクタ
    Window(int width = 640, int height = 480, const char *title = "Hello!")
    : window(glfwCreateWindow(width, height, title, NULL, NULL))
    , scale(100.0f), location{ 0.0f, 0.0f }, keyStatus(GLFW_RELEASE)
    {
        if (window == NULL) {
            // ウィンドウが作成できなかった
            std::cerr << "Can't create GLFW window." << std::endl;
            exit(1);
        }
        
        // 現在のウィンドウを処理対象にする
        glfwMakeContextCurrent(window);

        // GLEW を初期化する
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            // GLEW の初期化に失敗
            std::cerr << "Can't initialize GLFW" << std::endl;
            exit(1);
        }

        // 垂直同期のタイミングを待つ
        glfwSwapInterval(1);

        // ウィンドウのサイズ変更時に呼び出す処理の登録
        glfwSetWindowSizeCallback(window, resize);

        // マウスホイール操作時に呼び出す処理の登録
        glfwSetScrollCallback(window, wheel);

        // キーボード操作時に呼び出す処理の登録
        glfwSetKeyCallback(window, keyboard);

        // このインスタンスの this ポインタを記録しておく
        glfwSetWindowUserPointer(window, this);

        // 開いたウィンドウの初期設定
        resize(window, width, height);
    }

    // デストラクタ
    virtual ~Window(){
        glfwDestroyWindow(window);
    }

    // 描画ループの継続判定
    explicit operator bool(){
        // イベントを取り出す
//        if (keyStatus == GLFW_RELEASE)
//            glfwWaitEvents();
//        else
//            glfwPollEvents();
        glfwPollEvents();

        // キーボードの状態を調べる
        if (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_RELEASE)
            location[0] -= 2.0f / size[0];
        else if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_RELEASE)
            location[0] += 2.0f / size[0];
        if (glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_RELEASE)
            location[1] -= 2.0f / size[1];
        else if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_RELEASE)
            location[1] += 2.0f / size[1];

        // マウスの左ボタンの状態を調べる
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) != GLFW_RELEASE) {
            // 左クリックされていたらマウスカーソルの位置を取得する
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            // マウスカーソルの正規化デバイス座標系上での位置を求める
            location[0] = static_cast<GLfloat>(x) * 2.0f / size[0] - 1.0f;
            location[1] =  - (static_cast<GLfloat>(y) * 2.0f / size[1] - 1.0f);
        }

        // ウィンドウを閉じる必要がなければ true を返す
        return !glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE);
    }

    // ダブルバッファリング
    void swapBuffers() const{
        // カラーバッファを入れ替える
        glfwSwapBuffers(window);
    }

    // ウィンドウのサイズ変更時の処理
    static void resize(GLFWwindow *const window, int width, int height){
        // フレームバッファのサイズを調べる
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        // フレームバッファ全体をビューポートに設定する
        glViewport(0, 0, fbWidth, fbHeight);

        // このインスタンスの this ポインタを得る
        Window *const 
        instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

        if (instance != NULL) {
            // 開いたウィンドウのサイズを保持する
            instance->size[0] = static_cast<GLfloat>(width);
            instance->size[1] = static_cast<GLfloat>(height);
        }
    }

    // マウスホイール操作時の処理
    static void wheel(GLFWwindow *const window, double x, double y){
        // このインスタンスの this ポインタを得る
        Window *const
        instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

        if (instance != NULL) {
            // ワールド座標系に対するデバイス座標系の拡大率を更新する
            instance->scale += static_cast<GLfloat>(y);
        }
    }

    // キーボード操作時の処理
    static void keyboard(GLFWwindow *const window, int key, int scancode, int action, int mods){
        // このインスタンスの this ポインタを得る
        Window *const
        instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

        if (instance != NULL) {
            // キーの状態を保存する
            instance->keyStatus = action;
        }
    }

    // ウィンドウのサイズを取り出す
    const GLfloat *getSize() const { return size; }

    //ワールド座標系に対するデバイス座標系の拡大率を取り出す
    GLfloat getScale() const { return scale; }

    // 位置を取り出す
    const GLfloat *getLocation() const { return location; }
};
