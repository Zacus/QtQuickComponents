#include <QtTest/QtTest>

#include "Yuv420ShaderUniforms.h"

#include <QMatrix4x4>

#include <rhi/qrhi_platform.h>

#include <memory>

namespace {

struct ResourceUpdateBatchReleaser
{
    void operator()(QRhiResourceUpdateBatch* updates) const
    {
        if (updates)
            updates->release();
    }
};

using ResourceUpdateBatchPtr = std::unique_ptr<QRhiResourceUpdateBatch, ResourceUpdateBatchReleaser>;

} // namespace

class Yuv420ShaderUniformsTest : public QObject
{
    Q_OBJECT

private slots:
    void buildsDefaultBt601UniformBlock();
    void uploadsUniformBufferAndTracksState();
    void rejectsInvalidInputs();
};

void Yuv420ShaderUniformsTest::buildsDefaultBt601UniformBlock()
{
    QMatrix4x4 transform;
    transform.translate(10.0f, 20.0f);
    const Yuv420ShaderUniforms::UniformBlock block =
        Yuv420ShaderUniforms::makeUniformBlock(0.75f, transform);

    QCOMPARE(sizeof(Yuv420ShaderUniforms::UniformBlock), size_t(144));
    QCOMPARE(block.transform[0], 1.0f);
    QCOMPARE(block.transform[5], 1.0f);
    QCOMPARE(block.transform[10], 1.0f);
    QCOMPARE(block.transform[12], 10.0f);
    QCOMPARE(block.transform[13], 20.0f);
    QCOMPARE(block.opacity[0], 0.75f);
    QCOMPARE(block.opacity[1], 0.0f);
    QCOMPARE(block.yuvToRgb[0], 1.164383f);
    QCOMPARE(block.yuvToRgb[2], 1.596027f);
    QVERIFY(qAbs(block.yuvToRgb[3] - -0.874202f) < 0.0001f);
    QCOMPARE(block.yuvToRgb[5], -0.391762f);
    QCOMPARE(block.yuvToRgb[6], -0.812968f);
    QVERIFY(qAbs(block.yuvToRgb[7] - 0.531668f) < 0.0001f);
    QCOMPARE(block.yuvToRgb[9], 2.017232f);
    QVERIFY(qAbs(block.yuvToRgb[11] - -1.085631f) < 0.0001f);
    QCOMPARE(block.yuvToRgb[15], 1.0f);
}

void Yuv420ShaderUniformsTest::uploadsUniformBufferAndTracksState()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    Yuv420ShaderUniforms uniforms;
    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    QVERIFY(uniforms.upload(rhi.get(), updates.get(), 0.5f, 11));
    QVERIFY(uniforms.isValid());
    QVERIFY(uniforms.buffer() != nullptr);
    QCOMPARE(uniforms.buffer()->type(), QRhiBuffer::Dynamic);
    QVERIFY(uniforms.buffer()->usage().testFlag(QRhiBuffer::UniformBuffer));
    QCOMPARE(uniforms.buffer()->size(), quint32(sizeof(Yuv420ShaderUniforms::UniformBlock)));
    QCOMPARE(uniforms.uploadedSerial(), quint64(11));
    QCOMPARE(uniforms.opacity(), 0.5f);

    QRhiBuffer* buffer = uniforms.buffer();
    QVERIFY(uniforms.upload(rhi.get(), updates.get(), 0.5f, 11));
    QCOMPARE(uniforms.buffer(), buffer);

    QVERIFY(uniforms.upload(rhi.get(), updates.get(), 0.25f, 11));
    QCOMPARE(uniforms.buffer(), buffer);
    QCOMPARE(uniforms.opacity(), 0.25f);

    QMatrix4x4 transform;
    transform.scale(2.0f, 3.0f);
    QVERIFY(uniforms.upload(rhi.get(), updates.get(), transform, 0.25f, 11));
    QCOMPARE(uniforms.buffer(), buffer);
    QCOMPARE(uniforms.uniformBlock().transform[0], 2.0f);
    QCOMPARE(uniforms.uniformBlock().transform[5], 3.0f);

    uniforms.release();
    QVERIFY(!uniforms.isValid());
    QCOMPARE(uniforms.buffer(), nullptr);
    QCOMPARE(uniforms.uploadedSerial(), quint64(0));
}

void Yuv420ShaderUniformsTest::rejectsInvalidInputs()
{
    Yuv420ShaderUniforms uniforms;
    ResourceUpdateBatchPtr updates;

    QVERIFY(!uniforms.upload(nullptr, updates.get(), 1.0f, 1));
    QVERIFY(!uniforms.upload(nullptr, updates.get(), 1.0f, 0));
    QVERIFY(!uniforms.isValid());
}

QTEST_APPLESS_MAIN(Yuv420ShaderUniformsTest)

#include "tst_yuv420_shader_uniforms.moc"
