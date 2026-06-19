#pragma once

#include "GlobalVideoRenderer.h"

#include <QRectF>
#include <QSGRenderNode>
#include <QSize>

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

    void setRect(const QRectF& rect);
    QRectF rect() const override;
    RenderingFlags flags() const override;
    void render(const RenderState* state) override;

private:
    void updateTexturePlan();

    GlobalVideoRenderer::Yuv420Snapshot m_snapshot;
    QRectF m_rect;
    QSize m_yTextureSize;
    QSize m_uTextureSize;
    QSize m_vTextureSize;
    quint64 m_uploadedSerial = 0;
};
