#pragma once

#include <QVector>

#include <rhi/qshader.h>
#include <rhi/qrhi.h>

class Yuv420ShaderPipeline
{
public:
    struct Vertex
    {
        float x = 0.0f;
        float y = 0.0f;
        float u = 0.0f;
        float v = 0.0f;
    };

    bool loadShaders();
    bool hasShaders() const;

    QShader vertexShader() const { return m_vertexShader; }
    QShader fragmentShader() const { return m_fragmentShader; }
    QVector<QRhiShaderStage> shaderStages() const;

    static QRhiVertexInputLayout vertexInputLayout();
    static QRhiGraphicsPipeline::Topology topology();
    static QVector<Vertex> quadVertices();

private:
    QShader m_vertexShader;
    QShader m_fragmentShader;
};
