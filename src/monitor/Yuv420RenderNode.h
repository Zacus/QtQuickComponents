#pragma once

#include "GlobalVideoRenderer.h"
#include "Yuv420ShaderUniforms.h"
#include "Yuv420TextureSet.h"

#include <QRectF>
#include <QSGRenderNode>
#include <QSize>

class QRhiResourceUpdateBatch;

class Yuv420RenderNode : public QSGRenderNode
{
public:
    Yuv420RenderNode(const GlobalVideoRenderer::Yuv420Snapshot& snapshot, const QRectF& rect);

    void setSnapshot(const GlobalVideoRenderer::Yuv420Snapshot& snapshot);
    const GlobalVideoRenderer::Yuv420Snapshot& snapshot() const { return m_snapshot; }
    quint64 serial() const { return m_snapshot.serial; }
    quint64 uploadedSerial() const { return m_uploadedSerial; }

    QSize yTextureSize() const;
    QSize uTextureSize() const;
    QSize vTextureSize() const;
    bool hasPendingTextureUpload() const;
    bool markTextureUploadCompleteForCurrentSnapshot();
    bool ensureTextureResources(QRhi* rhi);
    bool hasTextureResources() const;
    bool ensureShaderResources(QRhi* rhi);
    bool hasShaderResources() const;
    bool uploadPendingTextureData(QRhi* rhi, QRhiResourceUpdateBatch* updates);
    bool uploadShaderUniforms(QRhi* rhi, QRhiResourceUpdateBatch* updates, float opacity);
    bool hasShaderUniforms() const;

    void setRect(const QRectF& rect);
    QRectF rect() const override;
    RenderingFlags flags() const override;
    void render(const RenderState* state) override;
    void releaseResources() override;

private:
    void updateTexturePlan();

    GlobalVideoRenderer::Yuv420Snapshot m_snapshot;
    QRectF m_rect;
    QSize m_yTextureSize;
    QSize m_uTextureSize;
    QSize m_vTextureSize;
    quint64 m_uploadedSerial = 0;
    Yuv420TextureSet m_textures;
    Yuv420ShaderUniforms m_uniforms;
};
