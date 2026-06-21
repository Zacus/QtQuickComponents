#include "Yuv420ShaderPipeline.h"

#include <QFile>

namespace {

constexpr const char* VertexShaderPath = ":/QuickUI/Components/impl/shaders/Yuv420Blit.vert.qsb";
constexpr const char* FragmentShaderPath = ":/QuickUI/Components/impl/shaders/Yuv420Blit.frag.qsb";

QShader loadShader(const char* path)
{
    QFile file(QString::fromUtf8(path));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    return QShader::fromSerialized(file.readAll());
}

} // namespace

namespace QuickUI::Components::Internal {

bool Yuv420ShaderPipeline::loadShaders()
{
    m_vertexShader = loadShader(VertexShaderPath);
    m_fragmentShader = loadShader(FragmentShaderPath);
    return hasShaders();
}

bool Yuv420ShaderPipeline::hasShaders() const
{
    return m_vertexShader.isValid() && m_fragmentShader.isValid();
}

QVector<QRhiShaderStage> Yuv420ShaderPipeline::shaderStages() const
{
    if (!hasShaders())
        return {};

    return {
        QRhiShaderStage(QRhiShaderStage::Vertex, m_vertexShader),
        QRhiShaderStage(QRhiShaderStage::Fragment, m_fragmentShader),
    };
}

QRhiVertexInputLayout Yuv420ShaderPipeline::vertexInputLayout()
{
    QRhiVertexInputLayout layout;
    layout.setBindings({
        QRhiVertexInputBinding(sizeof(Vertex)),
    });
    layout.setAttributes({
        QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0),
        QRhiVertexInputAttribute(0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float)),
    });
    return layout;
}

QRhiGraphicsPipeline::Topology Yuv420ShaderPipeline::topology()
{
    return QRhiGraphicsPipeline::TriangleStrip;
}

QVector<Yuv420ShaderPipeline::Vertex> Yuv420ShaderPipeline::quadVertices()
{
    return {
        {-1.0f, -1.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 0.0f},
    };
}

QVector<Yuv420ShaderPipeline::Vertex> Yuv420ShaderPipeline::quadVertices(const QRectF& rect)
{
    const float left = float(rect.left());
    const float right = float(rect.right());
    const float top = float(rect.top());
    const float bottom = float(rect.bottom());

    return {
        {left, bottom, 0.0f, 1.0f},
        {right, bottom, 1.0f, 1.0f},
        {left, top, 0.0f, 0.0f},
        {right, top, 1.0f, 0.0f},
    };
}

} // namespace QuickUI::Components::Internal
