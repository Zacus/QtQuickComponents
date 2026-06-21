#pragma once

#include "Yuv420Vertex.h"

#include <QRectF>
#include <QVector>

#include <rhi/qshader.h>
#include <rhi/qrhi.h>

class Yuv420ShaderPipeline
{
public:
    using Vertex = Yuv420Vertex;

    bool loadShaders();
    bool hasShaders() const;

    QShader vertexShader() const { return m_vertexShader; }
    QShader fragmentShader() const { return m_fragmentShader; }
    QVector<QRhiShaderStage> shaderStages() const;

    static QRhiVertexInputLayout vertexInputLayout();
    static QRhiGraphicsPipeline::Topology topology();
    static QVector<Vertex> quadVertices();
    static QVector<Vertex> quadVertices(const QRectF& rect);

private:
    QShader m_vertexShader;
    QShader m_fragmentShader;
};
