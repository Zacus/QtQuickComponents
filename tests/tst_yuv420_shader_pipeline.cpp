#include <QtTest/QtTest>

#include "Yuv420ShaderPipeline.h"

#include <QRectF>

class Yuv420ShaderPipelineTest : public QObject
{
    Q_OBJECT

private slots:
    void loadsShaderBytecodeFromResources();
    void exposesFixedVertexInputLayout();
    void exposesFullscreenTriangleStripVertices();
    void exposesLocalRectTriangleStripVertices();
};

void Yuv420ShaderPipelineTest::loadsShaderBytecodeFromResources()
{
    Yuv420ShaderPipeline pipeline;

    QVERIFY(pipeline.loadShaders());
    QVERIFY(pipeline.hasShaders());
    QVERIFY(pipeline.vertexShader().isValid());
    QVERIFY(pipeline.fragmentShader().isValid());

    const QVector<QRhiShaderStage> stages = pipeline.shaderStages();
    QCOMPARE(stages.size(), 2);
    QCOMPARE(stages.at(0).type(), QRhiShaderStage::Vertex);
    QCOMPARE(stages.at(1).type(), QRhiShaderStage::Fragment);
}

void Yuv420ShaderPipelineTest::exposesFixedVertexInputLayout()
{
    const QRhiVertexInputLayout layout = Yuv420ShaderPipeline::vertexInputLayout();

    QCOMPARE(layout.bindingCount(), qsizetype(1));
    QCOMPARE(layout.bindingAt(0)->stride(), quint32(sizeof(Yuv420ShaderPipeline::Vertex)));

    QCOMPARE(layout.attributeCount(), qsizetype(2));
    QCOMPARE(layout.attributeAt(0)->binding(), 0);
    QCOMPARE(layout.attributeAt(0)->location(), 0);
    QCOMPARE(layout.attributeAt(0)->format(), QRhiVertexInputAttribute::Float2);
    QCOMPARE(layout.attributeAt(0)->offset(), quint32(0));

    QCOMPARE(layout.attributeAt(1)->binding(), 0);
    QCOMPARE(layout.attributeAt(1)->location(), 1);
    QCOMPARE(layout.attributeAt(1)->format(), QRhiVertexInputAttribute::Float2);
    QCOMPARE(layout.attributeAt(1)->offset(), quint32(2 * sizeof(float)));
}

void Yuv420ShaderPipelineTest::exposesFullscreenTriangleStripVertices()
{
    QCOMPARE(Yuv420ShaderPipeline::topology(), QRhiGraphicsPipeline::TriangleStrip);

    const auto vertices = Yuv420ShaderPipeline::quadVertices();
    QCOMPARE(vertices.size(), qsizetype(4));
    QCOMPARE(vertices.at(0).x, -1.0f);
    QCOMPARE(vertices.at(0).y, -1.0f);
    QCOMPARE(vertices.at(0).u, 0.0f);
    QCOMPARE(vertices.at(0).v, 1.0f);
    QCOMPARE(vertices.at(3).x, 1.0f);
    QCOMPARE(vertices.at(3).y, 1.0f);
    QCOMPARE(vertices.at(3).u, 1.0f);
    QCOMPARE(vertices.at(3).v, 0.0f);
}

void Yuv420ShaderPipelineTest::exposesLocalRectTriangleStripVertices()
{
    const auto vertices = Yuv420ShaderPipeline::quadVertices(QRectF(10.0, 20.0, 30.0, 40.0));

    QCOMPARE(vertices.size(), qsizetype(4));
    QCOMPARE(vertices.at(0).x, 10.0f);
    QCOMPARE(vertices.at(0).y, 60.0f);
    QCOMPARE(vertices.at(0).u, 0.0f);
    QCOMPARE(vertices.at(0).v, 1.0f);
    QCOMPARE(vertices.at(1).x, 40.0f);
    QCOMPARE(vertices.at(1).y, 60.0f);
    QCOMPARE(vertices.at(2).x, 10.0f);
    QCOMPARE(vertices.at(2).y, 20.0f);
    QCOMPARE(vertices.at(3).x, 40.0f);
    QCOMPARE(vertices.at(3).y, 20.0f);
    QCOMPARE(vertices.at(3).u, 1.0f);
    QCOMPARE(vertices.at(3).v, 0.0f);
}

QTEST_APPLESS_MAIN(Yuv420ShaderPipelineTest)

#include "tst_yuv420_shader_pipeline.moc"
