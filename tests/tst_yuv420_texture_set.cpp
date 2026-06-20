#include <QtTest/QtTest>

#include "Yuv420TextureSet.h"

#include <rhi/qrhi_platform.h>

#include <memory>

class Yuv420TextureSetTest : public QObject
{
    Q_OBJECT

private slots:
    void createsReusesAndReleasesPlaneTextures();
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

void Yuv420TextureSetTest::rejectsInvalidInputs()
{
    Yuv420TextureSet textures;

    QVERIFY(!textures.ensureTextures(nullptr, QSize(4, 4), QSize(2, 2), QSize(2, 2)));
    QVERIFY(!textures.isValid());
}

QTEST_APPLESS_MAIN(Yuv420TextureSetTest)

#include "tst_yuv420_texture_set.moc"
