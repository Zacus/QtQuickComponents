#pragma once

#include "GlobalVideoRenderer.h"

#include <QRectF>
#include <QSGRenderNode>

class Yuv420RenderNode : public QSGRenderNode
{
public:
    Yuv420RenderNode(const GlobalVideoRenderer::Yuv420Snapshot& snapshot, const QRectF& rect);

    void setSnapshot(const GlobalVideoRenderer::Yuv420Snapshot& snapshot);
    const GlobalVideoRenderer::Yuv420Snapshot& snapshot() const { return m_snapshot; }
    quint64 serial() const { return m_snapshot.serial; }

    void setRect(const QRectF& rect);
    QRectF rect() const override;
    RenderingFlags flags() const override;
    void render(const RenderState* state) override;

private:
    GlobalVideoRenderer::Yuv420Snapshot m_snapshot;
    QRectF m_rect;
};
