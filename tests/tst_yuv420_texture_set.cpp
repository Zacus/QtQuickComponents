#include <QtTest/QtTest>

#include "GlobalVideoRenderer.h"
#include "Yuv420ShaderUniforms.h"
#include "Yuv420TextureSet.h"

#include <rhi/qrhi_platform.h>

#include <memory>

using QuickUI::Components::Internal::Yuv420ShaderUniforms;
using QuickUI::Components::Internal::Yuv420TextureSet;

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

class Yuv420TextureSetTest : public QObject
{
    Q_OBJECT

private slots:
    void createsReusesAndReleasesPlaneTextures();
    void uploadsFramePlanesAndTracksSerial();
    void createsShaderResourceBindingsForPlanes();
    void rejectsInvalidInputs();
};

void Yuv420TextureSetTest::createsReusesAndReleasesPlaneTextures()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    Yuv420TextureSet textures;
    QVERIFY(textures.ensureTextures(rhi.get(), QSize(4, 6), QSize(2, 3), QSize(2, 3)));

    QVERIFY(textures.isValid());
    QCOMPARE(textures.rhi(), rhi.get());
    QCOMPARE(textures.yTextureSize(), QSize(4, 6));
    QCOMPARE(textures.uTextureSize(), QSize(2, 3));
    QCOMPARE(textures.vTextureSize(), QSize(2, 3));
    QVERIFY(textures.yTexture() != nullptr);
    QVERIFY(textures.uTexture() != nullptr);
    QVERIFY(textures.vTexture() != nullptr);
    QCOMPARE(textures.yTexture()->format(), QRhiTexture::R8);
    QCOMPARE(textures.yTexture()->pixelSize(), QSize(4, 6));

    QRhiTexture* yTexture = textures.yTexture();
    QVERIFY(textures.ensureTextures(rhi.get(), QSize(4, 6), QSize(2, 3), QSize(2, 3)));
    QCOMPARE(textures.yTexture(), yTexture);

    QVERIFY(textures.ensureTextures(rhi.get(), QSize(8, 4), QSize(4, 2), QSize(4, 2)));
    QCOMPARE(textures.yTextureSize(), QSize(8, 4));
    QCOMPARE(textures.yTexture()->pixelSize(), QSize(8, 4));

    textures.release();
    QVERIFY(!textures.isValid());
    QCOMPARE(textures.rhi(), nullptr);
    QCOMPARE(textures.yTexture(), nullptr);
}

void Yuv420TextureSetTest::uploadsFramePlanesAndTracksSerial()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = 4;
    frame.height = 4;
    frame.yStride = 8;
    frame.uStride = 4;
    frame.vStride = 4;
    frame.yPlane = QByteArray(28, char(16));
    frame.uPlane = QByteArray(6, char(128));
    frame.vPlane = QByteArray(6, char(128));

    Yuv420TextureSet textures;
    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);

    QVERIFY(textures.uploadFrame(rhi.get(), updates.get(), frame, 7));
    QCOMPARE(textures.uploadedSerial(), quint64(7));
    QCOMPARE(textures.yTextureSize(), QSize(4, 4));
    QCOMPARE(textures.uTextureSize(), QSize(2, 2));
    QCOMPARE(textures.vTextureSize(), QSize(2, 2));
    QVERIFY(textures.isValid());

    QRhiTexture* yTexture = textures.yTexture();
    QVERIFY(textures.uploadFrame(rhi.get(), updates.get(), frame, 7));
    QCOMPARE(textures.uploadedSerial(), quint64(7));
    QCOMPARE(textures.yTexture(), yTexture);
}

void Yuv420TextureSetTest::createsShaderResourceBindingsForPlanes()
{
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    QVERIFY(rhi != nullptr);

    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = 4;
    frame.height = 4;
    frame.yStride = 4;
    frame.uStride = 2;
    frame.vStride = 2;
    frame.yPlane = QByteArray(16, char(16));
    frame.uPlane = QByteArray(4, char(128));
    frame.vPlane = QByteArray(4, char(128));

    Yuv420TextureSet textures;
    Yuv420ShaderUniforms uniforms;
    ResourceUpdateBatchPtr updates(rhi->nextResourceUpdateBatch());
    QVERIFY(updates != nullptr);
    QVERIFY(textures.uploadFrame(rhi.get(), updates.get(), frame, 9));
    QVERIFY(uniforms.upload(rhi.get(), updates.get(), 1.0f, 9));

    QVERIFY(textures.ensureShaderResources(rhi.get(), uniforms.buffer()));
    QVERIFY(textures.shaderResourceBindings() != nullptr);
    QVERIFY(textures.sampler() != nullptr);
    QCOMPARE(textures.sampler()->magFilter(), QRhiSampler::Linear);
    QCOMPARE(textures.sampler()->minFilter(), QRhiSampler::Linear);
    QCOMPARE(textures.sampler()->addressU(), QRhiSampler::ClampToEdge);
    QCOMPARE(textures.shaderResourceBindings()->bindingCount(), qsizetype(4));

    const QVector<quint32> layout = textures.shaderResourceBindings()->serializedLayoutDescription();
    const quint32 uniformStages = quint32(QRhiShaderResourceBinding::VertexStage)
        | quint32(QRhiShaderResourceBinding::FragmentStage);
    const QVector<quint32> expectedLayout = {
        0, uniformStages, quint32(QRhiShaderResourceBinding::UniformBuffer), 1,
        1, quint32(QRhiShaderResourceBinding::FragmentStage), quint32(QRhiShaderResourceBinding::SampledTexture), 1,
        2, quint32(QRhiShaderResourceBinding::FragmentStage), quint32(QRhiShaderResourceBinding::SampledTexture), 1,
        3, quint32(QRhiShaderResourceBinding::FragmentStage), quint32(QRhiShaderResourceBinding::SampledTexture), 1,
    };
    QCOMPARE(layout, expectedLayout);

    const quint64 bindingsId = textures.shaderResourceBindings()->globalResourceId();
    QVERIFY(textures.ensureShaderResources(rhi.get(), uniforms.buffer()));
    QCOMPARE(textures.shaderResourceBindings()->globalResourceId(), bindingsId);

    frame.width = 8;
    frame.height = 4;
    frame.yStride = 8;
    frame.uStride = 4;
    frame.vStride = 4;
    frame.yPlane = QByteArray(32, char(16));
    frame.uPlane = QByteArray(8, char(128));
    frame.vPlane = QByteArray(8, char(128));
    QVERIFY(textures.uploadFrame(rhi.get(), updates.get(), frame, 10));
    QVERIFY(uniforms.upload(rhi.get(), updates.get(), 1.0f, 10));
    QVERIFY(textures.ensureShaderResources(rhi.get(), uniforms.buffer()));
    QVERIFY(textures.shaderResourceBindings()->globalResourceId() != bindingsId);
}

void Yuv420TextureSetTest::rejectsInvalidInputs()
{
    Yuv420TextureSet textures;

    QVERIFY(!textures.ensureTextures(nullptr, QSize(4, 4), QSize(2, 2), QSize(2, 2)));
    QVERIFY(!textures.isValid());

    GlobalVideoRenderer::Yuv420Frame invalidFrame;
    ResourceUpdateBatchPtr updates;
    QVERIFY(!textures.uploadFrame(nullptr, updates.get(), invalidFrame, 1));
    QCOMPARE(textures.uploadedSerial(), quint64(0));
}

QTEST_APPLESS_MAIN(Yuv420TextureSetTest)

#include "tst_yuv420_texture_set.moc"
