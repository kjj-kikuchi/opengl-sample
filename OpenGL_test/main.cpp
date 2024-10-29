//
//  main.cpp
//  OpenGL_test
//
//  Created by 菊池祐作 on 2024/04/25.
//

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Window.h"
#include "Matrix.h"
#include "Shape.h"
#include "ShapeIndex.h"
#include "SolidShapeIndex.h"
#include "SolidShape.h"
#include "Mesh.h"

// シェーダオブジェクトのコンパイル結果を表示
// shader : シェーダオブジェクト名
// str : コンパイルエラーが発生した場所を示す文字列
GLboolean printShaderInfoLog(GLuint shader, const char *str){
    //コンパイル結果を取得
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) std::cerr << "Compile Error in " << str << std::endl;

    // シェーダのコンパイル時のログの長さを取得
    GLsizei bufSize;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);

    if (bufSize > 1) {
        // シェーダのコンパイル時のログの内容を取得
        std::vector<GLchar> infoLog(bufSize);
        GLsizei length;
        glGetShaderInfoLog(shader, bufSize, &length, &infoLog[0]);
        std::cerr << &infoLog[0] << std::endl;
    }

    return static_cast<GLboolean>(status);
}

// プログラムオブジェクトのリンク結果を表示
// program : プログラムオブジェクト名
// str : コンパイルエラーが発生した場所を示す文字列
GLboolean printProgramInfoLog(GLuint program){
    //コンパイル結果を取得
    GLint status;
    glGetShaderiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) std::cerr << "Link Error." << std::endl;

    // シェーダのコンパイル時のログの長さを取得
    GLsizei bufSize;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufSize);

    if (bufSize > 1) {
        // シェーダのコンパイル時のログの内容を取得
        std::vector<GLchar> infoLog(bufSize);
        GLsizei length;
        glGetProgramInfoLog(program, bufSize, &length, &infoLog[0]);
        std::cerr << &infoLog[0] << std::endl;
    }

    return static_cast<GLboolean>(status);
}

// プログラムオブジェクトを作成
// vsrc : バーテックスシェーダのソースプログラムの文字列
// fsrc : フラグメントシェーダのソースプログラムの文字列
GLuint createProgram(const char *vsrc, const char *fsrc){
    // 空のプログラムオブジェクトを作成
    const GLuint program(glCreateProgram());

    if (vsrc != NULL) {
        // バーテックスシェーダのシェーダオブジェクトを作成
        const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
        glShaderSource(vobj, 1, &vsrc, NULL);
        glCompileShader(vobj);

        // バーテックスシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
        if (printShaderInfoLog(vobj, "vertex shader")) {
            glAttachShader(program, vobj);
            glDeleteShader(vobj);
        }
    }

    if (fsrc != NULL) {
        // フラグメントシェーダのシェーダオブジェクトを作成
        const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
        glShaderSource(fobj, 1, &fsrc, NULL);
        glCompileShader(fobj);

        // バーテックスシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
        if (printShaderInfoLog(fobj, "fragment shader")) {
            glAttachShader(program, fobj);
            glDeleteShader(fobj);
        }
    }

    // プログラムオブジェクトをリンク
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "normal");
    glBindFragDataLocation(program, 0, "fragment");
    glLinkProgram(program);

    // 作成したプログラムオブジェクトを返す
    if (printProgramInfoLog(program)) {
        return program;
    }

    // プログラムオブジェクトが作成できなければ 0 を返す
    glDeleteProgram(program);
    return 0;
}

// シェーダのソースファイルを読み込んだメモリを返す
// name : シェーダのソースファイル名
// buffer : 読み込んだソースファイルのテキスト
bool readShaderSource(const char *name, std::vector<GLchar> &buffer){
    // ファイル名がNULL
    if (name == NULL) return false;

    // ソースファイルを開く
    std::ifstream file(name, std::ios::binary);
    if (file.fail()) {
        std::cerr << "Error: Can't open source file: " << name << std::endl;
        return false;
    }

    // ファイルの末尾に移動し現在位置（＝ファイルサイズ）を得る
    file.seekg(0L, std::ios::end);
    GLsizei length = static_cast<GLsizei>(file.tellg());

    // ファイルサイズのメモリを確保
    buffer.resize(length + 1);

    // ファイルを先頭から読み込む
    file.seekg(0L, std::ios::beg);
    file.read(buffer.data(), length);
    buffer[length] = '\0';

    if (file.fail()) {
        std::cerr << "Error: Could not read source file: " << name << std::endl;
        file.close();
        return false;
    }

    // 読み込み成功
    file.close();
    return true;
}

// シェーダのソースファイルを読み込んでプログラムオブジェクトを作成する
// vert : バーテックスシェーダのソースファイル名
// frag : フラグメントシェーダのソースファイル名
GLuint loadProgram(const char *vert, const char *frag){
    // シェーダのソースファイルを読み込む
    std::vector<GLchar> vsrc;
    const bool vstat(readShaderSource(vert, vsrc));
    std::vector<GLchar> fsrc;
    const bool fstat(readShaderSource(frag, fsrc));

    // プログラムオブジェクトを作成
    return  vstat && fstat ? createProgram(vsrc.data(), fsrc.data()) : 0;
}

// 六面体の頂点の位置
//constexpr Object::Vertex cubeVertex[] =
//{
//    { -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f },    // (0)
//    { -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.8f },     // (1)
//    { -1.0f,  1.0f,  1.0f,  0.0f,  0.8f,  0.0f },      // (2)
//    { -1.0f,  1.0f, -1.0f,  0.0f,  0.8f,  0.8f  },     // (3)
//    {  1.0f,  1.0f, -1.0f,  0.8f,  0.0f,  0.8f  },      // (4)
//    {  1.0f, -1.0f, -1.0f,  0.8f,  0.0f,  0.8f  },     // (5)
//    {  1.0f, -1.0f,  1.0f,  0.8f,  0.8f,  0.0f  },      // (6)
//    {  1.0f,  1.0f,  1.0f,  0.8f,  0.8f,  0.8f  }        // (7)
//};
//
//// 六面体の稜線の両端点のインデックス
//constexpr GLuint wireCubeIndex[] =
//{
//    1, 0, // (a)
//    2, 7, // (b)
//    3, 0, // (c)
//    4, 7, // (d)
//    5, 0, // (e)
//    6, 7, // (f)
//    1, 2, // (g)
//    2, 3, // (h)
//    3, 4, // (i)
//    4, 5, // (j)
//    5, 6, // (k)
//    6, 1  // (l)
//};
//
//// 面ごとに色を変えた六面体の頂点属性
//constexpr Object::Vertex solidCubeVertex[] =
//{
//    // 左
//    { -1.0f, -1.0f, -1.0f, 0.1f, 0.8f, 0.1f },
//    { -1.0f, -1.0f, 1.0f, 0.1f, 0.8f, 0.1f },
//    { -1.0f, 1.0f, 1.0f, 0.1f, 0.8f, 0.1f },
//    { -1.0f, -1.0f, -1.0f, 0.1f, 0.8f, 0.1f },
//    { -1.0f, 1.0f, 1.0f, 0.1f, 0.8f, 0.1f },
//    { -1.0f, 1.0f, -1.0f, 0.1f, 0.8f, 0.1f },
//    // 裏
//    { 1.0f, -1.0f, -1.0f, 0.8f, 0.1f, 0.8f },
//    { -1.0f, -1.0f, -1.0f, 0.8f, 0.1f, 0.8f },
//    { -1.0f, 1.0f, -1.0f, 0.8f, 0.1f, 0.8f },
//    { 1.0f, -1.0f, -1.0f, 0.8f, 0.1f, 0.8f },
//    { -1.0f, 1.0f, -1.0f, 0.8f, 0.1f, 0.8f },
//    { 1.0f, 1.0f, -1.0f, 0.8f, 0.1f, 0.8f },
//    // 下
//    { -1.0f, -1.0f, -1.0f, 0.1f, 0.8f, 0.8f },
//    { 1.0f, -1.0f, -1.0f, 0.1f, 0.8f, 0.8f },
//    { 1.0f, -1.0f, 1.0f, 0.1f, 0.8f, 0.8f },
//    { -1.0f, -1.0f, -1.0f, 0.1f, 0.8f, 0.8f },
//    { 1.0f, -1.0f, 1.0f, 0.1f, 0.8f, 0.8f },
//    { -1.0f, -1.0f, 1.0f, 0.1f, 0.8f, 0.8f },
//    // 右
//    { 1.0f, -1.0f, 1.0f, 0.1f, 0.1f, 0.8f },
//    { 1.0f, -1.0f, -1.0f, 0.1f, 0.1f, 0.8f },
//    { 1.0f, 1.0f, -1.0f, 0.1f, 0.1f, 0.8f },
//    { 1.0f, -1.0f, 1.0f, 0.1f, 0.1f, 0.8f },
//    { 1.0f, 1.0f, -1.0f, 0.1f, 0.1f, 0.8f },
//    { 1.0f, 1.0f, 1.0f, 0.1f, 0.1f, 0.8f },
//    // 上
//    { -1.0f, 1.0f, -1.0f, 0.8f, 0.1f, 0.1f },
//    { -1.0f, 1.0f, 1.0f, 0.8f, 0.1f, 0.1f },
//    { 1.0f, 1.0f, 1.0f, 0.8f, 0.1f, 0.1f },
//    { -1.0f, 1.0f, -1.0f, 0.8f, 0.1f, 0.1f },
//    { 1.0f, 1.0f, 1.0f, 0.8f, 0.1f, 0.1f },
//    { 1.0f, 1.0f, -1.0f, 0.8f, 0.1f, 0.1f },
//    // 前
//    { -1.0f, -1.0f, 1.0f, 0.8f, 0.8f, 0.1f },
//    { 1.0f, -1.0f, 1.0f, 0.8f, 0.8f, 0.1f },
//    { 1.0f, 1.0f, 1.0f, 0.8f, 0.8f, 0.1f },
//    { -1.0f, -1.0f, 1.0f, 0.8f, 0.8f, 0.1f },
//    { 1.0f, 1.0f, 1.0f, 0.8f, 0.8f, 0.1f },
//    { -1.0f, 1.0f, 1.0f, 0.8f, 0.8f, 0.1f }
//};
//
//// 六面体の面を塗りつぶす三角形の頂点のインデックス
//constexpr GLuint solidCubeIndex[] =
//{
//    0, 1, 2, 3, 4, 5, // 左
//    6, 7, 8, 9,10,11,//裏
//    12, 13, 14, 15, 16, 17, //下
//    18, 19, 20, 21, 22, 23, // 右
//    24, 25, 26, 27, 28, 29, // 上
//    30,31,32,33,34,35 //前
//};

int main(int argc, const char * argv[])
{
    // GLFWを初期化
    if (glfwInit() == GL_FALSE) {
        // 初期化に失敗
        std::cerr << "Can't initialize GLFW" << std::endl;
        return 1;
    }

    // プログラム終了時の処理を登録
    atexit(glfwTerminate);

    // OpenGL Version 3.2 Core Profile を選択
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ウィンドウを作成
    Window window;

    // 背景色を指定
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    // 背面カリングを有効にする
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // デプスバッファを有効にする
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // プログラムオブジェクトを作成
    const GLuint program(loadProgram("point.vert", "point.frag"));

    // uniform 変数の場所を取得する
    const GLint modelviewLoc(glGetUniformLocation(program, "modelview"));
    const GLint projectionLoc(glGetUniformLocation(program, "projection"));



    // 図形データを作成する

    //std::unique_ptr<const Shape> shape(new SolidShapeIndex(3, 36, solidCubeVertex, 36, solidCubeIndex));

    // メッシュを読み込み，データを作成
    std::string filename;
    if (argc != 2){
        std::cout << "command line error\n";
        std::exit(1);
    }
    filename = std::string(argv[1]);
    Mesh mesh;
    mesh.reedOBJ(filename);
    Object::Vertex Vertices[mesh.getVertexSize()];
    GLuint Indices[mesh.getIndexSize()];
    mesh.convertMeshData(Vertices, Indices);
    //mesh.exportOBJ(filename);
    std::unique_ptr<const Shape> meshShape(new SolidShapeIndex(3, mesh.getVertexSize(), Vertices, mesh.getIndexSize(), Indices));





    // タイマーを 0 にセット
    glfwSetTime(0.0);

    // ウィンドウが開いている間繰り返す
    while (window) {
        // ウィンドウを消去
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // シェーダプログラムの使用開始
        glUseProgram(program);

        // 直交投影変換行列を求める
        //const GLfloat *const size(window.getSize());
        //const GLfloat scale(window.getScale() * 2.0f);
        //const GLfloat w(size[0] / scale), h(size[1] / scale);
        //const Matrix projection(Matrix::orthogonal(-w, w, -h, h, 1.0f, 10.0f));

        // 透視投影変換行列を求める
        const GLfloat *const size(window.getSize());
        const GLfloat fovy(window.getScale() * 0.01f);
        const GLfloat aspect(size[0] / size[1]);
        const Matrix projection(Matrix::perspective(fovy, aspect, 1.0f, 10.0f));

        // 拡大縮小の変換行列を求める
//        const GLfloat scale(window.getScale() * 2.0f);
//        const Matrix scaling(Matrix::scale(scale / size[0], scale / size[1], 1.0f));

        // 平行移動の変換行列を求める
//        const GLfloat *const position(window.getLocation());
//        const Matrix translation(Matrix::translate(position[0], position[1], 0.0f));

        // モデル変換行列を求める
        const GLfloat *const location(window.getLocation());
//        const Matrix model(Matrix::translate(location[0], location[1], 0.0f));

        //（回転）
        const Matrix r(Matrix::rotate(static_cast<GLfloat>(glfwGetTime()), 0.0f, 1.0f, 0.0f));
        const Matrix model(Matrix::translate(location[0], location[1], 0.0f) * r);

        // ビュー変換行列を求める
//        const Matrix view(Matrix::lookat(3.0f, 4.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));
//        const Matrix view(Matrix::lookat(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));
        const Matrix view(Matrix::lookat(2.0f, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));

        // モデルビュー変換行列を求める
        const Matrix modelview(view * model);

        // uniform 変数に値を設定する
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.data());
        glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, modelview.data());

        // 図形を描画する
        //shape->draw();
        meshShape->draw();

        
        /*
        // 二つ目のモデルビュー変換行列を求める
        const Matrix modelview1(modelview * Matrix::translate(0.0f, 0.0f, 3.0f));

        // uniform 変数に値を設定する
        glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, modelview1.data());
        
        // 二つ目の図形を描画する
        //shape->draw();
        */

        // カラーバッファを入れ替える
        window.swapBuffers();
    }
}
