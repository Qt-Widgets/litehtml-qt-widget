#include "container_qt5.h"
#include "types.h"
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopServices>

#include "graphicscontext.h"

#include "fontcache.h"
#include "pathqt.h"
#include <cmath>

// https://github.com/Nanquitas/3DS_eBook_Reader/blob/51f1fedc2565de36253104a01f4689db00c35991/source/Litehtml3DSContainer.cpp#L18
#define PPI 132.1

static litehtml::position::vector	m_clips;

typedef std::map<QByteArray, QImage*> images_map;

static int totalElements = 0;

//static QString m_base_url = "://res/";

static images_map m_images;

//static QImage m_broken_img;

static QRect getRect(const litehtml::position &position) {
  return QRect(position.x, position.y, position.width, position.height);
}

static QColor getColor(const litehtml::web_color &color) {
  return QColor(color.red, color.green, color.blue, color.alpha);
}

static void	apply_clip( QPainter* pnt )
{
  if(!m_clips.empty())
  {
    litehtml::position clip_pos = m_clips.back();
    //pnt->setClipRect( clip_pos.left(), clip_pos.top(), clip_pos.width, clip_pos.height );
  }
  /*

  for(const auto& clip_box : m_clips)
  {
    rounded_rectangle(cr, clip_box.box, clip_box.radius);
    cairo_clip(cr);
  }
  */
}

static QString make_url( const char* url, const char* basepath, QUrl* pUrl = nullptr )
{
  /*QUrl	u = QUrl::fromUserInput( url, (basepath && basepath[0]) ? basepath : m_base_url );
  if( pUrl )
    *pUrl = u;
  return u.toLocalFile();*/
  return QString(url) + QString(basepath);
}

static QImage* getImage( const litehtml::tchar_t* src,  const litehtml::tchar_t* baseurl )
{
  QString fname = make_url( src, baseurl );
  images_map::iterator img = m_images.find(fname.toUtf8());
  if(img != m_images.end()) {
    return img->second;
  }

  //return &m_broken_img;
  return nullptr;
}

static bool shouldCreateElement(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr< litehtml::document >& doc) {
  Q_UNUSED(tag_name);
  Q_UNUSED(attributes);
  Q_UNUSED(doc);
  return true;
}

static void setPenForBorder(QPainter *painter, const litehtml::border &border)
{
    QPen pen(painter->pen());
    pen.setWidth(border.width);
    pen.setColor(getColor(border.color));
    /*
    border_style_none,
    border_style_hidden,
    border_style_dotted,
    border_style_dashed,
    border_style_solid,
    border_style_double,
    border_style_groove,
    border_style_ridge,
    border_style_inset,
    border_style_outset
    */
    switch (border.style) {
        case litehtml::border_style_none:
        case litehtml::border_style_hidden: pen.setStyle(Qt::NoPen); break;
        case litehtml::border_style_dotted: pen.setStyle(Qt::DotLine); break;
        case litehtml::border_style_dashed: pen.setStyle(Qt::DashLine); break;
        default: pen.setStyle(Qt::SolidLine); break;
    }
    painter->setPen(pen);
}

// Now for the real stuff

int container_qt5::m_defaultFontSize = 12;

container_qt5::container_qt5(QWidget* parent)
    : QObject(parent), litehtml::document_container()
{
    m_owner = parent;
    Q_ASSERT(m_owner != nullptr);
}

container_qt5::~container_qt5()
{

}

int container_qt5::getDefaultFontSize() {
  return m_defaultFontSize;
}

/*void container_qt5::setDrawArea(const QRect &area)
{
  m_drawArea = area;
}

QRect container_qt5::getDrawArea() const {
  return m_drawArea;
}*/

void container_qt5::set_document(std::shared_ptr< litehtml::document > doc)
{
    _doc = doc;
}

void container_qt5::setScroll(const QPoint& val) {
  m_Scroll = val;
}

void container_qt5::setScrollX(const int& val) {
  m_Scroll.setX(val);
}

void container_qt5::setScrollY(const int& val) {
  m_Scroll.setY(val);
}

QPoint container_qt5::getScroll() const {
  return m_Scroll;
}

void container_qt5::repaint(QPainter& painter)
{
    //qDebug() << __FUNCTION__ << m_drawArea.width();

    painter.setRenderHint(QPainter::Antialiasing);

    QRect	rc = m_owner->rect();

    //_doc->render(m_drawArea.width());

    /// \note don`t allow render size < 1
    _doc->render(std::max(rc.width(), 1));

    litehtml::position clipPos;
    clipPos.width 	= rc.width();
    clipPos.height 	= rc.height();
    clipPos.x 		= rc.x();
    clipPos.y 		= rc.y();

    //GraphicsContext* graphicsContext = new GraphicsContext(&painter);

    // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebKit2/WebProcess/WebPage/qt/ChunkedUpdateDrawingAreaQt.cpp#L44
    GraphicsContext graphicsContext(&painter);

    Q_ASSERT(graphicsContext.platformContext() != nullptr);
    _doc->draw(static_cast<litehtml::uint_ptr>(&graphicsContext), getScroll().x(), getScroll().y(), &clipPos);

/*qDebug() << "m_lastDocWidth" << m_lastDocWidth;
qDebug() << "m_lastDocHeight" << m_lastDocHeight;
qDebug() << "_doc->width()" << _doc->width();
qDebug() << "_doc->height()" << _doc->height();*/

    if (_doc->width() != m_lastDocWidth || _doc->height() != m_lastDocHeight) {
      emit docSizeChanged(_doc->width(), _doc->height());
      m_lastDocWidth = _doc->width();
      m_lastDocHeight = _doc->height();
    }
}

void container_qt5::setLastMouseCoords(int x, int y, int xClient,int yClient) {
    lastCursorX = x;
    lastCursorY = y;
    lastCursorClientX = xClient;
    lastCursorClientY = yClient;
}

/*void container_qt5::setSize(int w, int h)
{
  m_drawArea.setWidth(w);
  m_drawArea.setHeight(h);
}*/

void container_qt5::get_language(litehtml::tstring& language, litehtml::tstring& culture) const
{
    qDebug() << "get_language";
    language = _t("en");
    culture = _t("");
}

void container_qt5::get_media_features(litehtml::media_features& media) const
{
    //qDebug() << "get_media_features";

    QRect	rc = m_owner->rect();

    media.type = litehtml::media_type_screen;
    media.width = rc.width();
    media.height = rc.height();
    media.color = 8;
    media.monochrome = 0;
    media.color_index = 256;
    media.resolution = 96;
    media.device_width = rc.width();
    media.device_height = rc.height();

    qDebug() << "=> " << media.width << "x" << media.height;
}

// see https://github.com/litehtml/litebrowser/blob/master/src/ToolbarWnd.cpp#L572
std::shared_ptr< litehtml::element > container_qt5::create_element(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr< litehtml::document >& doc)
{
    //qDebug() << __FUNCTION__ << " this one can be 0";
    //qDebug() << __FUNCTION__ << " this one can be 0";
    //doc->root()->get_child()
  if(!shouldCreateElement(tag_name, attributes, doc)) {
    return std::shared_ptr<litehtml::element>();
  }

  /*if (!t_strcasecmp(tag_name, _t("input")))
  {
    return std::make_shared<container_el_inputbox>(doc, attributes);
  }*/

  litehtml::tstring attributeStr;
  for (auto attr : attributes)
  {
    attributeStr.append(attr.first);
    attributeStr.append(_t("="));
    attributeStr.append(attr.second);
    attributeStr.append(_t("\n"));
  }
  std::string _attributes = std::string(attributeStr.begin(), attributeStr.end());

  //ElementInfo elementInfo = {};
  int elementID = totalElements++;
  if (elementID > 0)
  {
    // TODO

    //std::shared_ptr<TagElement> result = std::make_shared(doc);
    //result->SetManagedInfo(elementInfo);
    //_elements[elementID] = result;
    //return std::shared_ptr<litehtml::element>(result.get());
    //return std::make_shared<litehtml::element>(doc);
    return std::shared_ptr<litehtml::element>();
  }

  return std::shared_ptr<litehtml::element>();
}

void container_qt5::get_client_rect(litehtml::position& client) const
{
    /*//qDebug() << "get_client_rect";
    // No scroll yet
    client.move_to(0, 0);
    client.width = m_drawArea.width();
    client.height = m_drawArea.height();

    //qDebug() << "==> " << client.width << "x" << client.height;*/

    QRect	rc = m_owner->rect();
    client.x = rc.left();
    client.y = rc.top();
    client.width = rc.width();
    client.height = rc.height();
}

// Deletes the last clipping.
void container_qt5::del_clip()
{
    //qDebug() << "del_clip";
    if(!m_clips.empty())
    {
      m_clips.pop_back();
      //if(!m_clips.empty())
      //{
      //	litehtml::position clip_pos = m_clips.back();
      //}
    }
}

// Set the painting clip rectangle here. valid_x and valid_y are ignored.
// Please note, litehtml can set some clip rects.
// You have to save the clip positions and apply clipping on draw something.
void container_qt5::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y)
{
  Q_UNUSED(pos);
  Q_UNUSED(bdr_radius);
  Q_UNUSED(bdr_radius);
  Q_UNUSED(valid_x);
  Q_UNUSED(valid_y);
    //qDebug() << "set_clip";

    litehtml::position clip_pos = pos;
    QRect	rc = m_owner->rect( );

    if(!valid_x)
    {
      clip_pos.x		= 0;//rc.left();
      clip_pos.width	= rc.width();
    }

    if(!valid_y)
    {
      clip_pos.y		= 0;//rc.top();
      clip_pos.height	= rc.height();
    }

    m_clips.push_back( clip_pos );

}

void container_qt5::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
  QFile qFile(url.c_str());

  if(!qFile.exists())
  {
    qDebug() << "nonexistent file " << qFile.fileName();
    return;
  }

  if(!qFile.open(QIODevice::ReadOnly))
  {
     qDebug() << "Can`t read data from " << qFile.fileName();
     return;
  }

  //QByteArray fileData = qFile.readAll();
  m_loaded_css[QString(url.c_str())] = qFile.readAll();
  QByteArray& fileData = m_loaded_css[QString(url.c_str())];

  if(!fileData.size())
  {
    qDebug() << "Empty data from " << qFile.fileName();
    return;
  }

  qFile.close();

  text = (const char*)&fileData.data()[0];

  //qDebug() << "imported css" << url.c_str();
}

void container_qt5::transform_text(litehtml::tstring& text, litehtml::text_transform tt)
{
  //qDebug() << "transform_text";

// TODO: UTF8 https://github.com/pinaraf/litehtml-qt/blob/f3f4959f1d4a4884a0a18d87db7483e886cc3ee3/containers/cairo/cairo_container.cpp#L942

/*
 * The text-transform property in CSS controls text case and capitalization.
    lowercase makes all of the letters in the selected text lowercase.
    uppercase makes all of the letters in the selected text uppercase.
    capitalize capitalizes the first letter of each word in the selected text.
    none leaves the text's case and capitalization exactly as it was entered.
    inherit gives the text the case and capitalization of its parent.
 */

  std::string temp = text;

  // capitalizes the first letter of each word
  if (tt == litehtml::text_transform_capitalize) {
    // Convert lowercase letter to uppercase
    // see http://www.cplusplus.com/reference/cctype/toupper/
    temp[0] = toupper(temp[0]); // TODO: do to each word

  } else if (tt == litehtml::text_transform_uppercase) {
    // std::transform applies the given function to a range and stores the result in another range
    // see https://en.cppreference.com/w/cpp/algorithm/transform
    std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);

  } else if (tt == litehtml::text_transform_lowercase) {
    // std::transform applies the given function to a range and stores the result in another range
    // see https://en.cppreference.com/w/cpp/algorithm/transform
    std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
  }

  text = temp.c_str();
}

void container_qt5::set_cursor(const litehtml::tchar_t* cursor_)
{
    /*QString cursor(cursor_);
    if (cursor == "auto")
        setCursor(Qt::IBeamCursor);
    else if (cursor == "pointer")
        setCursor(Qt::PointingHandCursor);
    else
        qDebug() << __FUNCTION__ << cursor;*/

    QString cursor(cursor_);

    QCursor	c( Qt::ArrowCursor );

    if( (cursor=="pointer") ) {
      c.setShape( Qt::PointingHandCursor );
    }

    m_owner->setCursor( c );
}

void container_qt5::on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el)
{
    //qDebug() << __FUNCTION__ << url;
    QDesktopServices::openUrl(QUrl(url));
}

void container_qt5::link(const std::shared_ptr< litehtml::document >& doc, const litehtml::element::ptr& el)
{
    //qDebug() << __FUNCTION__;
}

void container_qt5::set_base_url(const litehtml::tchar_t* base_url)
{
    //qDebug() << __FUNCTION__;
}

void container_qt5::set_caption(const litehtml::tchar_t* caption)
{
    //qDebug() << __FUNCTION__;
}

static Color toColor(const litehtml::web_color& clr) {
  return Color(clr.red,clr.green,clr.blue,clr.alpha);
}

static IntRect borderInnerRect(const IntRect& borderRect, unsigned short topWidth, unsigned short bottomWidth, unsigned short leftWidth, unsigned short rightWidth)
{
    return IntRect(
            borderRect.x() + leftWidth,
            borderRect.y() + topWidth,
            borderRect.width() - leftWidth - rightWidth,
            borderRect.height() - topWidth - bottomWidth);
}

static bool borderWillArcInnerEdge(const IntSize& firstRadius, const IntSize& secondRadius, int firstBorderWidth, int secondBorderWidth, int middleBorderWidth)
{
    // FIXME: This test is insufficient. We need to take border style into account.
    return (!firstRadius.width() || firstRadius.width() >= firstBorderWidth)
            && (!firstRadius.height() || firstRadius.height() >= middleBorderWidth)
            && (!secondRadius.width() || secondRadius.width() >= secondBorderWidth)
            && (!secondRadius.height() || secondRadius.height() >= middleBorderWidth);
}

static void drawLineForBoxSide(ColorSpace colorSpace, GraphicsContext* graphicsContext, int x1, int y1, int x2, int y2,
                                      BoxSide s, Color c, EBorderStyle style,
                                      int adjbw1, int adjbw2)
{
    int width = (s == BSTop || s == BSBottom ? y2 - y1 : x2 - x1);

    if (style == DOUBLE && width < 3)
        style = SOLID;

    switch (style) {
        case BNONE:
        case BHIDDEN:
            return;
        case DOTTED:
        case DASHED:
            graphicsContext->setStrokeColor(c, colorSpace);
            graphicsContext->setStrokeThickness(width);
            graphicsContext->setStrokeStyle(style == DASHED ? DashedStroke : DottedStroke);

            if (width > 0)
                switch (s) {
                    case BSBottom:
                    case BSTop:
                        graphicsContext->drawLine(IntPoint(x1, (y1 + y2) / 2), IntPoint(x2, (y1 + y2) / 2));
                        break;
                    case BSRight:
                    case BSLeft:
                        graphicsContext->drawLine(IntPoint((x1 + x2) / 2, y1), IntPoint((x1 + x2) / 2, y2));
                        break;
                }
            break;
        case DOUBLE: {
            int third = (width + 1) / 3;

            if (adjbw1 == 0 && adjbw2 == 0) {
                graphicsContext->setStrokeStyle(NoStroke);
                graphicsContext->setFillColor(c, colorSpace);
                switch (s) {
                    case BSTop:
                    case BSBottom:
                        graphicsContext->drawRect(IntRect(x1, y1, x2 - x1, third));
                        graphicsContext->drawRect(IntRect(x1, y2 - third, x2 - x1, third));
                        break;
                    case BSLeft:
                        graphicsContext->drawRect(IntRect(x1, y1 + 1, third, y2 - y1 - 1));
                        graphicsContext->drawRect(IntRect(x2 - third, y1 + 1, third, y2 - y1 - 1));
                        break;
                    case BSRight:
                        graphicsContext->drawRect(IntRect(x1, y1 + 1, third, y2 - y1 - 1));
                        graphicsContext->drawRect(IntRect(x2 - third, y1 + 1, third, y2 - y1 - 1));
                        break;
                }
            } else {
                int adjbw1bigthird = ((adjbw1 > 0) ? adjbw1 + 1 : adjbw1 - 1) / 3;
                int adjbw2bigthird = ((adjbw2 > 0) ? adjbw2 + 1 : adjbw2 - 1) / 3;

                switch (s) {
                    case BSTop:
                        drawLineForBoxSide(colorSpace, graphicsContext, x1 + max((-adjbw1 * 2 + 1) / 3, 0),
                                   y1, x2 - max((-adjbw2 * 2 + 1) / 3, 0), y1 + third,
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        drawLineForBoxSide(colorSpace, graphicsContext, x1 + max((adjbw1 * 2 + 1) / 3, 0),
                                   y2 - third, x2 - max((adjbw2 * 2 + 1) / 3, 0), y2,
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        break;
                    case BSLeft:
                        drawLineForBoxSide(colorSpace, graphicsContext, x1, y1 + max((-adjbw1 * 2 + 1) / 3, 0),
                                   x1 + third, y2 - max((-adjbw2 * 2 + 1) / 3, 0),
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        drawLineForBoxSide(colorSpace, graphicsContext, x2 - third, y1 + max((adjbw1 * 2 + 1) / 3, 0),
                                   x2, y2 - max((adjbw2 * 2 + 1) / 3, 0),
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        break;
                    case BSBottom:
                        drawLineForBoxSide(colorSpace, graphicsContext, x1 + max((adjbw1 * 2 + 1) / 3, 0),
                                   y1, x2 - max((adjbw2 * 2 + 1) / 3, 0), y1 + third,
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        drawLineForBoxSide(colorSpace, graphicsContext, x1 + max((-adjbw1 * 2 + 1) / 3, 0),
                                   y2 - third, x2 - max((-adjbw2 * 2 + 1) / 3, 0), y2,
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        break;
                    case BSRight:
                        drawLineForBoxSide(colorSpace, graphicsContext, x1, y1 + max((adjbw1 * 2 + 1) / 3, 0),
                                   x1 + third, y2 - max((adjbw2 * 2 + 1) / 3, 0),
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        drawLineForBoxSide(colorSpace, graphicsContext, x2 - third, y1 + max((-adjbw1 * 2 + 1) / 3, 0),
                                   x2, y2 - max((-adjbw2 * 2 + 1) / 3, 0),
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        case RIDGE:
        case GROOVE:
        {
            EBorderStyle s1;
            EBorderStyle s2;
            if (style == GROOVE) {
                s1 = INSET;
                s2 = OUTSET;
            } else {
                s1 = OUTSET;
                s2 = INSET;
            }

            int adjbw1bighalf = ((adjbw1 > 0) ? adjbw1 + 1 : adjbw1 - 1) / 2;
            int adjbw2bighalf = ((adjbw2 > 0) ? adjbw2 + 1 : adjbw2 - 1) / 2;

            switch (s) {
                case BSTop:
                    drawLineForBoxSide(colorSpace, graphicsContext, x1 + max(-adjbw1, 0) / 2, y1, x2 - max(-adjbw2, 0) / 2, (y1 + y2 + 1) / 2,
                               s, c, s1, adjbw1bighalf, adjbw2bighalf);
                    drawLineForBoxSide(colorSpace, graphicsContext, x1 + max(adjbw1 + 1, 0) / 2, (y1 + y2 + 1) / 2, x2 - max(adjbw2 + 1, 0) / 2, y2,
                               s, c, s2, adjbw1 / 2, adjbw2 / 2);
                    break;
                case BSLeft:
                    drawLineForBoxSide(colorSpace, graphicsContext, x1, y1 + max(-adjbw1, 0) / 2, (x1 + x2 + 1) / 2, y2 - max(-adjbw2, 0) / 2,
                               s, c, s1, adjbw1bighalf, adjbw2bighalf);
                    drawLineForBoxSide(colorSpace, graphicsContext, (x1 + x2 + 1) / 2, y1 + max(adjbw1 + 1, 0) / 2, x2, y2 - max(adjbw2 + 1, 0) / 2,
                               s, c, s2, adjbw1 / 2, adjbw2 / 2);
                    break;
                case BSBottom:
                    drawLineForBoxSide(colorSpace, graphicsContext, x1 + max(adjbw1, 0) / 2, y1, x2 - max(adjbw2, 0) / 2, (y1 + y2 + 1) / 2,
                               s, c, s2, adjbw1bighalf, adjbw2bighalf);
                    drawLineForBoxSide(colorSpace, graphicsContext, x1 + max(-adjbw1 + 1, 0) / 2, (y1 + y2 + 1) / 2, x2 - max(-adjbw2 + 1, 0) / 2, y2,
                               s, c, s1, adjbw1 / 2, adjbw2 / 2);
                    break;
                case BSRight:
                    drawLineForBoxSide(colorSpace, graphicsContext, x1, y1 + max(adjbw1, 0) / 2, (x1 + x2 + 1) / 2, y2 - max(adjbw2, 0) / 2,
                               s, c, s2, adjbw1bighalf, adjbw2bighalf);
                    drawLineForBoxSide(colorSpace, graphicsContext, (x1 + x2 + 1) / 2, y1 + max(-adjbw1 + 1, 0) / 2, x2, y2 - max(-adjbw2 + 1, 0) / 2,
                               s, c, s1, adjbw1 / 2, adjbw2 / 2);
                    break;
            }
            break;
        }
        case INSET:
            if (s == BSTop || s == BSLeft)
                c = c.dark();
            // fall through
        case OUTSET:
            if (style == OUTSET && (s == BSBottom || s == BSRight))
                c = c.dark();
            // fall through
        case SOLID: {
            graphicsContext->setStrokeStyle(NoStroke);
            graphicsContext->setFillColor(c, colorSpace);
            ASSERT(x2 >= x1);
            ASSERT(y2 >= y1);
            if (!adjbw1 && !adjbw2) {
                graphicsContext->drawRect(IntRect(x1, y1, x2 - x1, y2 - y1));
                return;
            }
            FloatPoint quad[4];
            switch (s) {
                case BSTop:
                    quad[0] = FloatPoint(x1 + max(-adjbw1, 0), y1);
                    quad[1] = FloatPoint(x1 + max(adjbw1, 0), y2);
                    quad[2] = FloatPoint(x2 - max(adjbw2, 0), y2);
                    quad[3] = FloatPoint(x2 - max(-adjbw2, 0), y1);
                    break;
                case BSBottom:
                    quad[0] = FloatPoint(x1 + max(adjbw1, 0), y1);
                    quad[1] = FloatPoint(x1 + max(-adjbw1, 0), y2);
                    quad[2] = FloatPoint(x2 - max(-adjbw2, 0), y2);
                    quad[3] = FloatPoint(x2 - max(adjbw2, 0), y1);
                    break;
                case BSLeft:
                    quad[0] = FloatPoint(x1, y1 + max(-adjbw1, 0));
                    quad[1] = FloatPoint(x1, y2 - max(-adjbw2, 0));
                    quad[2] = FloatPoint(x2, y2 - max(adjbw2, 0));
                    quad[3] = FloatPoint(x2, y1 + max(adjbw1, 0));
                    break;
                case BSRight:
                    quad[0] = FloatPoint(x1, y1 + max(adjbw1, 0));
                    quad[1] = FloatPoint(x1, y2 - max(adjbw2, 0));
                    quad[2] = FloatPoint(x2, y2 - max(-adjbw2, 0));
                    quad[3] = FloatPoint(x2, y1 + max(-adjbw1, 0));
                    break;
            }
            graphicsContext->drawConvexPolygon(4, quad);
            break;
        }
    }
}

static void clipBorderSidePolygon(GraphicsContext* graphicsContext, const IntRect& box,
  const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight,
  const BoxSide side, bool firstEdgeMatches, bool secondEdgeMatches,
  int borderLeftWidth, int borderRightWidth, int borderTopWidth, int borderBottomWidth
  /*const RenderStyle* style*/)
{
    FloatPoint quad[4];
    int tx = box.x();
    int ty = box.y();
    int w = box.width();
    int h = box.height();

    // For each side, create an array of FloatPoints where each point is based on whichever value in each corner
    // is larger -- the radius width/height or the border width/height -- as appropriate.
    switch (side) {
    case BSTop:
        quad[0] = FloatPoint(tx, ty);
        quad[1] = FloatPoint(
            tx + max(topLeft.width(), (int) borderLeftWidth),
            ty + max(topLeft.height(), (int) borderTopWidth));
        quad[2] = FloatPoint(
            tx + w - max(topRight.width(), (int) borderRightWidth),
            ty + max(topRight.height(), (int)borderTopWidth));
        quad[3] = FloatPoint(tx + w, ty);
        break;
    case BSLeft:
        quad[0] = FloatPoint(tx, ty);
        quad[1] = FloatPoint(
            tx + max(topLeft.width(), (int) borderLeftWidth),
            ty + max(topLeft.height(), (int) borderTopWidth));
        quad[2] = FloatPoint(
            tx + max(bottomLeft.width(), (int) borderLeftWidth),
            ty + h - max(bottomLeft.height(), (int)borderBottomWidth));
        quad[3] = FloatPoint(tx, ty + h);
        break;
    case BSBottom:
        quad[0] = FloatPoint(tx, ty + h);
        quad[1] = FloatPoint(
            tx + max(bottomLeft.width(), (int) borderLeftWidth),
            ty + h - max(bottomLeft.height(), (int)borderBottomWidth));
        quad[2] = FloatPoint(
            tx + w - max(bottomRight.width(), (int) borderRightWidth),
            ty + h - max(bottomRight.height(), (int)borderBottomWidth));
        quad[3] = FloatPoint(tx + w, ty + h);
        break;
    case BSRight:
        quad[0] = FloatPoint(tx + w, ty);
        quad[1] = FloatPoint(
            tx + w - max(topRight.width(), (int) borderRightWidth),
            ty + max(topRight.height(), (int) borderTopWidth));
        quad[2] = FloatPoint(
            tx + w - max(bottomRight.width(), (int) borderRightWidth),
            ty + h - max(bottomRight.height(), (int)borderBottomWidth));
        quad[3] = FloatPoint(tx + w, ty + h);
        break;
    default:
        break;
    }

    // If the border matches both of its adjacent sides, don't anti-alias the clip, and
    // if neither side matches, anti-alias the clip.
    if (firstEdgeMatches == secondEdgeMatches) {
        graphicsContext->clipConvexPolygon(4, quad, !firstEdgeMatches);
        return;
    }

    FloatPoint firstQuad[4];
    firstQuad[0] = quad[0];
    firstQuad[1] = quad[1];
    firstQuad[2] = side == BSTop || side == BSBottom ? FloatPoint(quad[3].x(), quad[2].y())
        : FloatPoint(quad[2].x(), quad[3].y());
    firstQuad[3] = quad[3];
    graphicsContext->clipConvexPolygon(4, firstQuad, !firstEdgeMatches);

    FloatPoint secondQuad[4];
    secondQuad[0] = quad[0];
    secondQuad[1] = side == BSTop || side == BSBottom ? FloatPoint(quad[0].x(), quad[1].y())
        : FloatPoint(quad[1].x(), quad[0].y());
    secondQuad[2] = quad[2];
    secondQuad[3] = quad[3];
    graphicsContext->clipConvexPolygon(4, secondQuad, !secondEdgeMatches);
}



static void constrainCornerRadiiForRect(const IntRect& r, IntSize& topLeft, IntSize& topRight, IntSize& bottomLeft, IntSize& bottomRight)
{
    // Constrain corner radii using CSS3 rules:
    // http://www.w3.org/TR/css3-background/#the-border-radius

    float factor = 1;
    unsigned radiiSum;

    // top
    radiiSum = static_cast<unsigned>(topLeft.width()) + static_cast<unsigned>(topRight.width()); // Casts to avoid integer overflow.
    if (radiiSum > static_cast<unsigned>(r.width()))
        factor = min(static_cast<float>(r.width()) / radiiSum, factor);

    // bottom
    radiiSum = static_cast<unsigned>(bottomLeft.width()) + static_cast<unsigned>(bottomRight.width());
    if (radiiSum > static_cast<unsigned>(r.width()))
        factor = min(static_cast<float>(r.width()) / radiiSum, factor);

    // left
    radiiSum = static_cast<unsigned>(topLeft.height()) + static_cast<unsigned>(bottomLeft.height());
    if (radiiSum > static_cast<unsigned>(r.height()))
        factor = min(static_cast<float>(r.height()) / radiiSum, factor);

    // right
    radiiSum = static_cast<unsigned>(topRight.height()) + static_cast<unsigned>(bottomRight.height());
    if (radiiSum > static_cast<unsigned>(r.height()))
        factor = min(static_cast<float>(r.height()) / radiiSum, factor);

    // Scale all radii by f if necessary.
    if (factor < 1) {
        // If either radius on a corner becomes zero, reset both radii on that corner.
        topLeft.scale(factor);
        if (!topLeft.width() || !topLeft.height())
            topLeft = IntSize();
        topRight.scale(factor);
        if (!topRight.width() || !topRight.height())
            topRight = IntSize();
        bottomLeft.scale(factor);
        if (!bottomLeft.width() || !bottomLeft.height())
            bottomLeft = IntSize();
        bottomRight.scale(factor);
        if (!bottomRight.width() || !bottomRight.height())
            bottomRight = IntSize();
    }
}

static void getBorderRadiiForRect(
  const IntRect& r,
  const litehtml::borders& borders,
  IntSize& topLeft, IntSize& topRight, IntSize& bottomLeft, IntSize& bottomRight)
{
    /*topLeft = IntSize(surround->border.topLeft().width().calcValue(r.width()), surround->border.topLeft().height().calcValue(r.height()));
    topRight = IntSize(surround->border.topRight().width().calcValue(r.width()), surround->border.topRight().height().calcValue(r.height()));

    bottomLeft = IntSize(surround->border.bottomLeft().width().calcValue(r.width()), surround->border.bottomLeft().height().calcValue(r.height()));
    bottomRight = IntSize(surround->border.bottomRight().width().calcValue(r.width()), surround->border.bottomRight().height().calcValue(r.height()));

    constrainCornerRadiiForRect(r, topLeft, topRight, bottomLeft, bottomRight);*/

    topLeft = IntSize(borders.radius.top_left_x, borders.radius.top_left_y);
    topRight = IntSize(borders.radius.top_right_x, borders.radius.top_right_y);

    bottomLeft = IntSize(borders.radius.bottom_left_x, borders.radius.bottom_left_y);
    bottomRight = IntSize(borders.radius.bottom_right_x, borders.radius.bottom_right_y);

    constrainCornerRadiiForRect(r, topLeft, topRight, bottomLeft, bottomRight);
}

static void getInnerBorderRadiiForRectWithBorderWidths(const IntRect& innerRect,
const litehtml::borders& borders,
unsigned short topWidth, unsigned short bottomWidth, unsigned short leftWidth, unsigned short rightWidth,
IntSize& innerTopLeft, IntSize& innerTopRight, IntSize& innerBottomLeft, IntSize& innerBottomRight)
//const
{
    /*innerTopLeft = IntSize(surround->border.topLeft().width().calcValue(innerRect.width()), surround->border.topLeft().height().calcValue(innerRect.height()));
    innerTopRight = IntSize(surround->border.topRight().width().calcValue(innerRect.width()), surround->border.topRight().height().calcValue(innerRect.height()));
    innerBottomLeft = IntSize(surround->border.bottomLeft().width().calcValue(innerRect.width()), surround->border.bottomLeft().height().calcValue(innerRect.height()));
    innerBottomRight = IntSize(surround->border.bottomRight().width().calcValue(innerRect.width()), surround->border.bottomRight().height().calcValue(innerRect.height()));


    innerTopLeft.setWidth(max(0, innerTopLeft.width() - leftWidth));
    innerTopLeft.setHeight(max(0, innerTopLeft.height() - topWidth));

    innerTopRight.setWidth(max(0, innerTopRight.width() - rightWidth));
    innerTopRight.setHeight(max(0, innerTopRight.height() - topWidth));

    innerBottomLeft.setWidth(max(0, innerBottomLeft.width() - leftWidth));
    innerBottomLeft.setHeight(max(0, innerBottomLeft.height() - bottomWidth));

    innerBottomRight.setWidth(max(0, innerBottomRight.width() - rightWidth));
    innerBottomRight.setHeight(max(0, innerBottomRight.height() - bottomWidth));

    constrainCornerRadiiForRect(innerRect, innerTopLeft, innerTopRight, innerBottomLeft, innerBottomRight);
*/
    innerTopLeft = IntSize(borders.radius.top_left_x, borders.radius.top_left_y);
    innerTopRight = IntSize(borders.radius.top_right_x, borders.radius.top_right_y);
    innerBottomLeft = IntSize(borders.radius.bottom_left_x, borders.radius.bottom_left_y);
    innerBottomRight = IntSize(borders.radius.bottom_right_x, borders.radius.bottom_right_y);

    innerTopLeft.setWidth(max(0, innerTopLeft.width() - leftWidth));
    innerTopLeft.setHeight(max(0, innerTopLeft.height() - topWidth));

    innerTopRight.setWidth(max(0, innerTopRight.width() - rightWidth));
    innerTopRight.setHeight(max(0, innerTopRight.height() - topWidth));

    innerBottomLeft.setWidth(max(0, innerBottomLeft.width() - leftWidth));
    innerBottomLeft.setHeight(max(0, innerBottomLeft.height() - bottomWidth));

    innerBottomRight.setWidth(max(0, innerBottomRight.width() - rightWidth));
    innerBottomRight.setHeight(max(0, innerBottomRight.height() - bottomWidth));

    constrainCornerRadiiForRect(innerRect, innerTopLeft, innerTopRight, innerBottomLeft, innerBottomRight);
}

static void drawBoxSideFromPath(GraphicsContext* graphicsContext, IntRect borderRect, Path borderPath,
                                    float thickness, float drawThickness,
                                    BoxSide s,
                                    const litehtml::borders& borders,
                                    //const RenderStyle* style,
                                    ColorSpace colorSpace,
                                    Color c, EBorderStyle borderStyle)
{
    if (thickness <= 0)
        return;

    if (borderStyle == DOUBLE && thickness < 3)
        borderStyle = SOLID;

    switch (borderStyle) {
    case BNONE:
    case BHIDDEN:
        return;
    case DOTTED:
    case DASHED: {
        graphicsContext->setStrokeColor(c, colorSpace);

        // The stroke is doubled here because the provided path is the
        // outside edge of the border so half the stroke is clipped off.
        // The extra multiplier is so that the clipping mask can antialias
        // the edges to prevent jaggies.
        graphicsContext->setStrokeThickness(drawThickness * 2 * 1.1f);
        graphicsContext->setStrokeStyle(borderStyle == DASHED ? DashedStroke : DottedStroke);

        // If the number of dashes that fit in the path is odd and non-integral then we
        // will have an awkwardly-sized dash at the end of the path. To try to avoid that
        // here, we simply make the whitespace dashes ever so slightly bigger.
        // FIXME: This could be even better if we tried to manipulate the dash offset
        // and possibly the whiteSpaceWidth to get the corners dash-symmetrical.
        float patWidth = thickness * ((borderStyle == DASHED) ? 3.0f : 1.0f);
        float whiteSpaceWidth = patWidth;
        float numberOfDashes = borderPath.length() / patWidth;
        bool evenNumberOfFullDashes = !((int)numberOfDashes % 2);
        bool integralNumberOfDashes = !(numberOfDashes - (int)numberOfDashes);
        if (!evenNumberOfFullDashes && !integralNumberOfDashes) {
            float numberOfWhitespaceDashes = numberOfDashes / 2;
            whiteSpaceWidth += (patWidth  / numberOfWhitespaceDashes);
        }

        DashArray lineDash;
        lineDash.append(patWidth);
        lineDash.append(whiteSpaceWidth);
        graphicsContext->setLineDash(lineDash, patWidth);
        graphicsContext->addPath(borderPath);
        graphicsContext->strokePath();
        return;
    }
    case DOUBLE: {
        int outerBorderTopWidth = borders.top.width / 3;
        int outerBorderRightWidth = borders.right.width / 3;
        int outerBorderBottomWidth = borders.bottom.width / 3;
        int outerBorderLeftWidth = borders.left.width / 3;

        int innerBorderTopWidth = borders.top.width * 2 / 3;
        int innerBorderRightWidth = borders.right.width * 2 / 3;
        int innerBorderBottomWidth = borders.bottom.width * 2 / 3;
        int innerBorderLeftWidth = borders.left.width * 2 / 3;

        // We need certain integer rounding results
        if (borders.top.width % 3 == 2)
            outerBorderTopWidth += 1;
        if (borders.right.width % 3 == 2)
            outerBorderRightWidth += 1;
        if (borders.bottom.width % 3 == 2)
            outerBorderBottomWidth += 1;
        if (borders.left.width % 3 == 2)
            outerBorderLeftWidth += 1;

        if (borders.top.width % 3 == 1)
            innerBorderTopWidth += 1;
        if (borders.right.width % 3 == 1)
            innerBorderRightWidth += 1;
        if (borders.bottom.width % 3 == 1)
            innerBorderBottomWidth += 1;
        if (borders.left.width % 3 == 1)
            innerBorderLeftWidth += 1;

        // Get the inner border rects for both the outer border line and the inner border line
        IntRect outerBorderInnerRect = borderInnerRect(borderRect, outerBorderTopWidth, outerBorderBottomWidth,
            outerBorderLeftWidth, outerBorderRightWidth);
        IntRect innerBorderInnerRect = borderInnerRect(borderRect, innerBorderTopWidth, innerBorderBottomWidth,
            innerBorderLeftWidth, innerBorderRightWidth);

        // Get the inner radii for the outer border line
        IntSize outerBorderTopLeftInnerRadius, outerBorderTopRightInnerRadius, outerBorderBottomLeftInnerRadius,
            outerBorderBottomRightInnerRadius;
        getInnerBorderRadiiForRectWithBorderWidths(outerBorderInnerRect, borders,
            outerBorderTopWidth, outerBorderBottomWidth,
            outerBorderLeftWidth, outerBorderRightWidth, outerBorderTopLeftInnerRadius, outerBorderTopRightInnerRadius,
            outerBorderBottomLeftInnerRadius, outerBorderBottomRightInnerRadius);

        // Get the inner radii for the inner border line
        IntSize innerBorderTopLeftInnerRadius, innerBorderTopRightInnerRadius, innerBorderBottomLeftInnerRadius,
            innerBorderBottomRightInnerRadius;
        getInnerBorderRadiiForRectWithBorderWidths(innerBorderInnerRect, borders,
            innerBorderTopWidth, innerBorderBottomWidth,
            innerBorderLeftWidth, innerBorderRightWidth, innerBorderTopLeftInnerRadius, innerBorderTopRightInnerRadius,
            innerBorderBottomLeftInnerRadius, innerBorderBottomRightInnerRadius);

        // Draw inner border line
        graphicsContext->save();
        graphicsContext->addRoundedRectClip(innerBorderInnerRect, innerBorderTopLeftInnerRadius,
            innerBorderTopRightInnerRadius, innerBorderBottomLeftInnerRadius, innerBorderBottomRightInnerRadius);
        drawBoxSideFromPath(graphicsContext, borderRect, borderPath, thickness, drawThickness, s, borders, colorSpace, c, SOLID);
        graphicsContext->restore();

        // Draw outer border line
        graphicsContext->save();
        graphicsContext->clipOutRoundedRect(outerBorderInnerRect, outerBorderTopLeftInnerRadius,
            outerBorderTopRightInnerRadius, outerBorderBottomLeftInnerRadius, outerBorderBottomRightInnerRadius);
        drawBoxSideFromPath(graphicsContext, borderRect, borderPath, thickness, drawThickness, s, borders, colorSpace, c, SOLID);
        graphicsContext->restore();

        return;
    }
    case RIDGE:
    case GROOVE:
    {
        EBorderStyle s1;
        EBorderStyle s2;
        if (borderStyle == GROOVE) {
            s1 = INSET;
            s2 = OUTSET;
        } else {
            s1 = OUTSET;
            s2 = INSET;
        }

        IntRect halfBorderRect = borderInnerRect(borderRect, borders.left.width / 2, borders.bottom.width / 2,
            borders.left.width / 2, borders.right.width / 2);

        IntSize topLeftHalfRadius, topRightHalfRadius, bottomLeftHalfRadius, bottomRightHalfRadius;
        getInnerBorderRadiiForRectWithBorderWidths(halfBorderRect, borders,
            borders.left.width / 2, borders.bottom.width / 2, borders.left.width / 2, borders.right.width / 2,
            topLeftHalfRadius, topRightHalfRadius, bottomLeftHalfRadius, bottomRightHalfRadius);

        // Paint full border
        drawBoxSideFromPath(graphicsContext, borderRect, borderPath, thickness, drawThickness, s, borders, colorSpace, c, s1);

        // Paint inner only
        graphicsContext->save();
        graphicsContext->addRoundedRectClip(halfBorderRect, topLeftHalfRadius, topRightHalfRadius,
            bottomLeftHalfRadius, bottomRightHalfRadius);
        drawBoxSideFromPath(graphicsContext, borderRect, borderPath, thickness, drawThickness, s, borders, colorSpace, c, s2);
        graphicsContext->restore();

        return;
    }
    case INSET:
        if (s == BSTop || s == BSLeft)
            c = c.dark();
        break;
    case OUTSET:
        if (s == BSBottom || s == BSRight)
            c = c.dark();
        break;
    default:
        break;
    }

    graphicsContext->setStrokeStyle(NoStroke);
    graphicsContext->setFillColor(c, colorSpace);
    graphicsContext->drawRect(borderRect);
}

static void paintBorder(
  ColorSpace colorSpace,
  const Color& topColor,  const Color& bottomColor,
  const Color& leftColor, const Color& rightColor,
  EBorderStyle topStyle,
  EBorderStyle bottomStyle,
  EBorderStyle leftStyle,
  EBorderStyle rightStyle,
  IntSize topLeftRadius, IntSize topRightRadius, IntSize bottomLeftRadius, IntSize bottomRightRadius,
  IntRect innerBorderRect,
  IntSize innerTopLeftRadius, IntSize innerTopRightRadius, IntSize innerBottomLeftRadius, IntSize innerBottomRightRadius,
  const litehtml::borders& borders,
  bool topTransparent, bool bottomTransparent, bool rightTransparent, bool leftTransparent,
  bool renderTop, bool renderLeft, bool renderRight, bool renderBottom,
  bool hasBorderRadius,
  GraphicsContext* graphicsContext,
  int tx, int ty, int w, int h,
  bool begin = true, bool end = true)
{
    /*if (paintNinePieceImage(graphicsContext, tx, ty, w, h, style, borderImage()))
        return;*/

    if (graphicsContext->paintingDisabled())
        return;

    bool renderRadii = false;
    Path roundedPath;
    IntSize topLeft, topRight, bottomLeft, bottomRight;
    IntRect borderRect(tx, ty, w, h);

    if (hasBorderRadius) {
        if (begin) {
            topLeft = topLeftRadius;
            bottomLeft = bottomLeftRadius;
        }
        if (end) {
            topRight = topRightRadius;
            bottomRight = bottomRightRadius;
        }

        renderRadii = true;

        // Clip to the inner and outer radii rects.
        graphicsContext->save();
        graphicsContext->addRoundedRectClip(borderRect, topLeft, topRight, bottomLeft, bottomRight);
        graphicsContext->clipOutRoundedRect(innerBorderRect, innerTopLeftRadius, innerTopRightRadius, innerBottomLeftRadius, innerBottomRightRadius);

        roundedPath.addRoundedRect(borderRect, topLeft, topRight, bottomLeft, bottomRight);
        graphicsContext->addPath(roundedPath);
    }

    bool upperLeftBorderStylesMatch = renderLeft && (topStyle == leftStyle) && (topColor == leftColor);
    bool upperRightBorderStylesMatch = renderRight && (topStyle == rightStyle) && (topColor == rightColor) && (topStyle != OUTSET) && (topStyle != RIDGE) && (topStyle != INSET) && (topStyle != GROOVE);
    bool lowerLeftBorderStylesMatch = renderLeft && (bottomStyle == leftStyle) && (bottomColor == leftColor) && (bottomStyle != OUTSET) && (bottomStyle != RIDGE) && (bottomStyle != INSET) && (bottomStyle != GROOVE);
    bool lowerRightBorderStylesMatch = renderRight && (bottomStyle == rightStyle) && (bottomColor == rightColor);

    if (renderTop) {
        int x = tx;
        int x2 = tx + w;

        if (renderRadii && borderWillArcInnerEdge(topLeft, topRight, borders.left.width, borders.right.width, borders.top.width)) {
            graphicsContext->save();
            clipBorderSidePolygon(graphicsContext, borderRect, topLeft, topRight, bottomLeft, bottomRight, BSTop, upperLeftBorderStylesMatch, upperRightBorderStylesMatch, borders.left.width, borders.right.width, borders.top.width, borders.bottom.width);
            float thickness = max(max(borders.top.width, borders.left.width), borders.right.width);
            drawBoxSideFromPath(graphicsContext, borderRect, roundedPath, borders.top.width, thickness, BSTop, borders, colorSpace, topColor, topStyle);
            graphicsContext->restore();
        } else {
            bool ignoreLeft = (topColor == leftColor && topTransparent == leftTransparent && topStyle >= OUTSET
                && (leftStyle == DOTTED || leftStyle == DASHED || leftStyle == SOLID || leftStyle == OUTSET));
            bool ignoreRight = (topColor == rightColor && topTransparent == rightTransparent && topStyle >= OUTSET
                && (rightStyle == DOTTED || rightStyle == DASHED || rightStyle == SOLID || rightStyle == INSET));

            drawLineForBoxSide(colorSpace, graphicsContext, x, ty, x2, ty + borders.top.width, BSTop, topColor, topStyle,
                    ignoreLeft ? 0 : borders.left.width, ignoreRight ? 0 : borders.right.width);
        }
    }

    if (renderBottom) {
        int x = tx;
        int x2 = tx + w;

        if (renderRadii && borderWillArcInnerEdge(bottomLeft, bottomRight, borders.left.width, borders.right.width, borders.bottom.width)) {
            graphicsContext->save();
            clipBorderSidePolygon(graphicsContext, borderRect, topLeft, topRight, bottomLeft, bottomRight, BSBottom, lowerLeftBorderStylesMatch, lowerRightBorderStylesMatch, borders.left.width, borders.right.width, borders.top.width, borders.bottom.width);
            float thickness = max(max(borders.bottom.width, borders.left.width), borders.right.width);
            drawBoxSideFromPath(graphicsContext, borderRect, roundedPath, borders.bottom.width, thickness, BSBottom, borders, colorSpace, bottomColor, bottomStyle);
            graphicsContext->restore();
        } else {
            bool ignoreLeft = (bottomColor == leftColor && bottomTransparent == leftTransparent && bottomStyle >= OUTSET
                && (leftStyle == DOTTED || leftStyle == DASHED || leftStyle == SOLID || leftStyle == OUTSET));

            bool ignoreRight = (bottomColor == rightColor && bottomTransparent == rightTransparent && bottomStyle >= OUTSET
                && (rightStyle == DOTTED || rightStyle == DASHED || rightStyle == SOLID || rightStyle == INSET));

            drawLineForBoxSide(colorSpace, graphicsContext, x, ty + h - borders.bottom.width, x2, ty + h, BSBottom, bottomColor,
                        bottomStyle, ignoreLeft ? 0 : borders.left.width,
                        ignoreRight ? 0 : borders.right.width);
        }
    }

    if (renderLeft) {
        int y = ty;
        int y2 = ty + h;

        if (renderRadii && borderWillArcInnerEdge(bottomLeft, topLeft, borders.bottom.width, borders.top.width, borders.left.width)) {
            graphicsContext->save();
            clipBorderSidePolygon(graphicsContext, borderRect, topLeft, topRight, bottomLeft, bottomRight, BSLeft, upperLeftBorderStylesMatch, lowerLeftBorderStylesMatch, borders.left.width, borders.right.width, borders.top.width, borders.bottom.width);
            float thickness = max(max(borders.left.width, borders.top.width), borders.bottom.width);
            drawBoxSideFromPath(graphicsContext, borderRect, roundedPath, borders.left.width, thickness, BSLeft, borders, colorSpace, leftColor, leftStyle);
            graphicsContext->restore();
        } else {
            bool ignoreTop = (topColor == leftColor && topTransparent == leftTransparent && leftStyle >= OUTSET
                && (topStyle == DOTTED || topStyle == DASHED || topStyle == SOLID || topStyle == OUTSET));

            bool ignoreBottom = (bottomColor == leftColor && bottomTransparent == leftTransparent && leftStyle >= OUTSET
                && (bottomStyle == DOTTED || bottomStyle == DASHED || bottomStyle == SOLID || bottomStyle == INSET));

            drawLineForBoxSide(colorSpace, graphicsContext, tx, y, tx + borders.left.width, y2, BSLeft, leftColor,
                        leftStyle, ignoreTop ? 0 : borders.top.width, ignoreBottom ? 0 : borders.bottom.width);
        }
    }

    if (renderRight) {
        if (renderRadii && borderWillArcInnerEdge(bottomRight, topRight, borders.bottom.width, borders.top.width, borders.right.width)) {
            graphicsContext->save();
            clipBorderSidePolygon(graphicsContext, borderRect, topLeft, topRight, bottomLeft, bottomRight, BSRight, upperRightBorderStylesMatch, lowerRightBorderStylesMatch, borders.left.width, borders.right.width, borders.top.width, borders.bottom.width);
            float thickness = max(max(borders.right.width, borders.top.width), borders.bottom.width);
            drawBoxSideFromPath(graphicsContext, borderRect, roundedPath, borders.right.width, thickness, BSRight, borders, colorSpace, rightColor, rightStyle);
            graphicsContext->restore();
        } else {
            bool ignoreTop = ((topColor == rightColor) && (topTransparent == rightTransparent)
                && (rightStyle >= DOTTED || rightStyle == INSET)
                && (topStyle == DOTTED || topStyle == DASHED || topStyle == SOLID || topStyle == OUTSET));

            bool ignoreBottom = ((bottomColor == rightColor) && (bottomTransparent == rightTransparent)
                && (rightStyle >= DOTTED || rightStyle == INSET)
                && (bottomStyle == DOTTED || bottomStyle == DASHED || bottomStyle == SOLID || bottomStyle == INSET));

            int y = ty;
            int y2 = ty + h;

            drawLineForBoxSide(colorSpace, graphicsContext, tx + w - borders.right.width, y, tx + w, y2, BSRight, rightColor,
                rightStyle, ignoreTop ? 0 : borders.top.width,
                ignoreBottom ? 0 : borders.bottom.width);
        }
    }

    if (renderRadii)
        graphicsContext->restore();
}

EBorderStyle toEBorderStyle(const litehtml::border& border) {
  switch (border.style) {
    case litehtml::border_style::border_style_none:
      return EBorderStyle::BNONE;
    case litehtml::border_style::border_style_inset:
      return EBorderStyle::INSET;
    case litehtml::border_style::border_style_ridge:
      return EBorderStyle::RIDGE;
    case litehtml::border_style::border_style_solid:
      return EBorderStyle::SOLID;
    case litehtml::border_style::border_style_dashed:
      return EBorderStyle::DASHED;
    case litehtml::border_style::border_style_dotted:
      return EBorderStyle::DOTTED;
    case litehtml::border_style::border_style_double:
      return EBorderStyle::DOUBLE;
    case litehtml::border_style::border_style_groove:
      return EBorderStyle::GROOVE;
    case litehtml::border_style::border_style_hidden:
      return EBorderStyle::BHIDDEN;
    case litehtml::border_style::border_style_outset:
      return EBorderStyle::OUTSET;
  }
}

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBoxModelObject.cpp#L1002
void container_qt5::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    if (root) {
      // we are in <html>, so no borders here
      return;
    }

    GraphicsContext* graphicsContext = (GraphicsContext*) hdc;
    if(!graphicsContext) {
      return;
    }
    QPainter *painter = (QPainter *) graphicsContext->platformContext();
    if(!painter) {
      return;
    }

    bool topTransparent = borders.top.color.alpha > 0;
    bool bottomTransparent = borders.bottom.color.alpha > 0;
    bool rightTransparent = borders.right.color.alpha > 0;
    bool leftTransparent = borders.left.color.alpha > 0;

    bool renderTop = borders.top.style > litehtml::border_style_hidden;
    bool renderLeft = borders.left.style > litehtml::border_style_hidden;
    bool renderRight = borders.right.style > litehtml::border_style_hidden;
    bool renderBottom = borders.bottom.style > litehtml::border_style_hidden;

    const Color& topColor = toColor(borders.top.color);
    const Color& bottomColor = toColor(borders.bottom.color);
    const Color& leftColor = toColor(borders.left.color);
    const Color& rightColor = toColor(borders.right.color);

    QRect  borderArea = getRect(draw_pos);

    IntRect borderRect(borderArea.x(), borderArea.y(), borderArea.width(), borderArea.height());

    IntSize topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius;
    getBorderRadiiForRect(borderRect, borders, topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius);

    IntRect innerBorderRect = borderInnerRect(borderRect, borders.top.width, borders.bottom.width,
              borders.left.width, borders.right.width);

    IntSize innerTopLeftRadius, innerTopRightRadius, innerBottomLeftRadius, innerBottomRightRadius;

    getInnerBorderRadiiForRectWithBorderWidths(innerBorderRect, borders,
      borders.top.width, borders.bottom.width, borders.left.width, borders.right.width,
      innerTopLeftRadius, innerTopRightRadius,
              innerBottomLeftRadius, innerBottomRightRadius);

    bool hasBorderRadius = true;

    graphicsContext->save();

    painter->setRenderHint(QPainter::Antialiasing);

    apply_clip( painter );

    paintBorder(sRGBColorSpace,
      topColor, bottomColor, leftColor, rightColor,
      toEBorderStyle(borders.top),
      toEBorderStyle(borders.bottom),
      toEBorderStyle(borders.left),
      toEBorderStyle(borders.right),
      topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius, // <<<
      innerBorderRect,
      innerTopLeftRadius, innerTopRightRadius, innerBottomLeftRadius, innerBottomRightRadius,
      borders,
      topTransparent, bottomTransparent, rightTransparent, leftTransparent,
      renderTop, renderLeft, renderRight, renderBottom,
      hasBorderRadius,
      graphicsContext,
      draw_pos.x, draw_pos.y, draw_pos.width, draw_pos.height, true, true
    );

    graphicsContext->restore();
}

void container_qt5::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg)
{
    Q_ASSERT(hdc);

    //qDebug() << "draw_background" << bg.color.red;

    GraphicsContext* graphicsContext = (GraphicsContext*) hdc;
    if(!graphicsContext) {
      return;
    }
    QPainter *painter = (QPainter *) graphicsContext->platformContext();
    if(!painter) {
      return;
    }

    // clip_box. Defines the position of the clipping box. See the background-clip CSS property.
    IntPoint clip_a(offsetX + bg.clip_box.left(), offsetY + bg.clip_box.top());
    IntPoint clip_b(offsetX + bg.clip_box.right(), offsetY + bg.clip_box.bottom());
    QRect clipRect(clip_a, clip_b);

    IntSize topLeft, topRight, bottomLeft, bottomRight;
    IntRect borderRect(clip_a.x(), clip_a.y(), clip_b.x(), clip_b.y());

    bool renderRadii = false;
    Path roundedPath;

#ifdef nope

    //QPainter* painter = (QPainter*)hdc;
    //QPainter ptr(m_owner);
    //QPainter* painter = &ptr;

    painter->save();

    apply_clip( painter );

    // clip_box. Defines the position of the clipping box. See the background-clip CSS property.
    QPoint clip_a(offsetX + bg.clip_box.left(), offsetY + bg.clip_box.top());
    QPoint clip_b(offsetX + bg.clip_box.right(), offsetY + bg.clip_box.bottom());
    QRect clipRect(clip_a, clip_b);

    if( clip_a.x() >= clip_b.x() || clip_a.y() >= clip_b.y() ) {
      // see example https://github.com/marcj/Pesto/blob/master/src/pesto/demo/example3.cpp
      painter->restore();
      return;
    }

    // The image contains the background image URL. URL can be relative.
    // Use the baseurl member to find the base URL for image.
    if (bg.image.empty())
    {
      // This member defines the background-color CSS property.
      painter->fillRect(clipRect, QBrush(getColor(bg.color)) );

      // border_box
      painter->fillRect(bg.border_box.x, bg.border_box.y, bg.border_box.width, bg.border_box.height, QBrush(getColor(bg.color)) );

      //pnt->fillRect(0, 0, 2000, 2000, QBrush(QColor(100,100,100,100)) );
      painter->restore();
      return;
    }


    // Defines the position of the origin box. See the background-origin CSS property.
    // The background-origin property specifies the origin position (the background positioning area) of a background image.
    // bg.origin_box
    // TODO https://github.com/PingmanTools/LiteHtmlSharp/blob/3f4db0ff0ab4b5cd7f5e10aa1b6ed94f2eee0bcb/LiteHtmlLib/src/DocContainer.cpp#L67


    /*
      This is the background-attachment CSS property. Can be one of the following values:

      background_attachment_scroll - CSS scroll
      background_attachment_fixed - - CSS fixed
    */
    if (bg.attachment == litehtml::background_attachment::background_attachment_fixed)
    {
      // TODO  - m_Scroll.y()
      painter->restore();
      return;
    }

    /// \note color used to fill rect if image not found/broken
    QColor dummyColor(100, 100, 100, 100);

    // TODO: https://github.com/nem0/lumixengine_html/blob/e2da43e704ad0ad474c8ecafc16fcae4a51e8aff/src/editor/plugins.cpp#L162
    //Crc32 crc32;
    //auto iter = m_images.find(crc32.calculateFromData(bg.image.c_str()));
    auto iter = m_images.find(bg.image.c_str());
    if (iter == m_images.end()) {
      //qDebug() << "iter == m_images.end()" << __FUNCTION__;
      // drw placeholder
      painter->fillRect(clipRect, dummyColor );
      return;
    }

    QImage* img = getImage( bg.image.c_str(), bg.baseurl.c_str() );
    if (!img) {
      painter->fillRect(clipRect, dummyColor );
      return;
    }

    //auto img = iter.value();

    /*
     * This the background-repeat CSS property. Can be one of the following values:

        background_repeat_repeat - CSS repeat
        background_repeat_repeat_x - CSS repeat-x
        background_repeat_repeat_y - CSS repeat-y
        background_repeat_no_repeat - CSS no-repeat
     */

    auto imscaled = img->scaled(clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    QBrush *brushTiledImage = new QBrush(imscaled);
    brushTiledImage->setStyle(Qt::BrushStyle::TexturePattern);

    switch (bg.repeat)
    {
      case litehtml::background_repeat_no_repeat:
      {
        //pnt->drawImage( bg.position_x, bg.position_y, *img, 0, 0, img->width(), img->height() );
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b);
        break;
      }
      case litehtml::background_repeat_repeat_x:
      {
        painter->setBackground(*brushTiledImage);
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //ImVec2 uv((clip_b.x - clip_a.x) / img.w, 0);
        //glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        //ImGui::GetWindowDrawList()->PushTextureID(img.textureId);
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b, ImVec2(0, 0), uv);
        //ImGui::GetWindowDrawList()->PopTextureID();
        break;
      }
      break;
      case litehtml::background_repeat_repeat_y:
      {
        painter->setBackground(*brushTiledImage);
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //ImVec2 uv(0, (clip_b.y - clip_a.y) / img.h);
        //glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        //ImGui::GetWindowDrawList()->PushTextureID(img.textureId);
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b, ImVec2(0, 0), uv);
        //ImGui::GetWindowDrawList()->PopTextureID();
        break;
      }
      case litehtml::background_repeat_repeat:
      {
        painter->setBackground(*brushTiledImage);
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //ImVec2 uv((clip_b.x - clip_a.x) / img.w, (clip_b.y - clip_a.y) / img.h);
        //glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        //ImGui::GetWindowDrawList()->PushTextureID(img.textureId);
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b, ImVec2(0, 0), uv);
        //ImGui::GetWindowDrawList()->PopTextureID();
        break;
      }
    }

    painter->restore();
#endif
}

void container_qt5::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz)
{
  //qDebug() << __FUNCTION__;
  QImage*	img = getImage( src, baseurl );
  if( img ) {
    sz.width = img->width();
    sz.height = img->height();
  } else {
    sz.width = sz.height = 100;
  }
}

void container_qt5::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready)
{
  //qDebug() << __FUNCTION__ << src << baseurl;

  QString fname = make_url( src, baseurl );

  if( m_images.find(fname.toUtf8())==m_images.end() ) {
    QImage*	img = new QImage( );
    if( img->load( fname ) ) {
      m_images[fname.toUtf8()] = img;
    }
    else {
      qWarning() << "cannot load image: " << fname;
    }
  }
}

void container_qt5::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    //qDebug() << __FUNCTION__;
    //qDebug() << marker.marker_type << marker.pos.x << "x" << marker.pos.y << marker.pos.left() << marker.pos.right();
    //qDebug() << marker.baseurl << QString::fromStdString(marker.image);

    //QPainter *painter = (QPainter *) hdc;

    GraphicsContext* graphicsContext = (GraphicsContext*) hdc;
    if(!graphicsContext) {
      return;
    }
    QPainter *painter = (QPainter *) graphicsContext->platformContext();
    if(!painter) {
      return;
    }

    painter->save();

    apply_clip( painter );

    QRect position = getRect(marker.pos);
    QColor color = getColor(marker.color);

    painter->setPen(color);
    switch (marker.marker_type) {
        case litehtml::list_style_type_none: break;
        case litehtml::list_style_type_circle:
          painter->setPen( QPen( color ) );
          painter->setBrush( Qt::NoBrush );
          painter->drawEllipse(position);
          break;
        case litehtml::list_style_type_disc:
          painter->setBrush(color);
          painter->drawEllipse(position);
          break;
        case litehtml::list_style_type_square:
          painter->fillRect(position, getColor(marker.color));
          break;
        // How to implement numeral markers ??
        default: break;
    }

    painter->restore();
}
/*
    list_style_type_none,
    list_style_type_circle,
    list_style_type_disc,
    list_style_type_square,
    list_style_type_armenian,
    list_style_type_cjk_ideographic,
    list_style_type_decimal,
    list_style_type_decimal_leading_zero,
    list_style_type_georgian,
    list_style_type_hebrew,
    list_style_type_hiragana,
    list_style_type_hiragana_iroha,
    list_style_type_katakana,
    list_style_type_katakana_iroha,
    list_style_type_lower_alpha,
    list_style_type_lower_greek,
    list_style_type_lower_latin,
    list_style_type_lower_roman,
    list_style_type_upper_alpha,
    list_style_type_upper_latin,
    list_style_type_upper_roman,
*/

const litehtml::tchar_t* container_qt5::get_default_font_name() const
{
    //qDebug() << __FUNCTION__;
    return FontCache::getInstance()->getDefaultFontName().toStdString().c_str();
}

int container_qt5::get_default_font_size() const
{
    //qDebug() << __FUNCTION__;
    return m_defaultFontSize;
}

int container_qt5::pt_to_px(int pt)
{
    //qDebug() << __FUNCTION__;
    return pt / 72 * PPI;
}

void container_qt5::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    //qDebug() << __FUNCTION__;
    //QPainter *painter = (QPainter *) hdc;

    GraphicsContext* graphicsContext = (GraphicsContext*) hdc;
    if(!graphicsContext) {
      return;
    }
    QPainter *painter = (QPainter *) graphicsContext->platformContext();
    if(!painter) {
      return;
    }

    painter->save();

    apply_clip(painter);

    QFont *font = (QFont *) hFont;
    //QFont *font = new QFont("Arial", 12, 12, false);
    painter->setFont(*font);
    //painter->setFont(QFont("Times",22));
    painter->setPen(getColor(color));
    //painter->setBrush(QColor(color.red, color.green, color.blue, color.alpha));
    QFontMetrics metrics(*font);
    painter->drawText(pos.x, pos.bottom() - metrics.descent(), text);

    //qDebug() << "Paint " << text << " at " << pos.x << "x" << pos.y;
    //QString text2 = QString::fromUtf8(qPrintable(u8"F?llungRaupeStepываываtext"));//tr("F?llungRaupeStepываываtext");
    //painter->drawText(pos.x, pos.bottom() - metrics.descent(), QString::fromUtf8(qPrintable(text)));
    //painter->drawText(pos.x, pos.bottom() - metrics.descent(), text2.toUtf8());
    //painter->drawText(pos.x, pos.bottom() - metrics.descent(), text2.toLatin1());
    //painter->drawText(pos.x, pos.bottom() - metrics.descent(), text2.toLocal8Bit());

    painter->restore();
}

int container_qt5::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont)
{
    if (!hFont) {
      qDebug() << __FUNCTION__ << " can`t get text width for empty font";
      return getDefaultFontSize();
    }

    //qDebug() << __FUNCTION__;
    QFont *font = (QFont *) hFont;
    QFontMetrics metrics(*font);
    QString txt(text);
    //qDebug() << "For" << txt << metrics.boundingRect(txt);
    /*if (txt == " ") {
        return metrics.boundingRect("x").width();
    }
    return metrics.boundingRect(txt).width();*/
    return metrics.width( text );
}

void container_qt5::delete_font(litehtml::uint_ptr hFont)
{
    //qDebug() << __FUNCTION__;
    QFont *font = (QFont *) hFont;
    delete(font);
}

litehtml::uint_ptr container_qt5::create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm)
{
    if (!faceName || !fm) {
      return nullptr;
    }

    //TODO: decoration
    qDebug() << __FUNCTION__ << " for " << faceName << size << weight;
    //QFont *font = new QFont("Arial Unicode MS", size, weight, italic == litehtml::fontStyleItalic);
    //QFont *font = new QFont(faceName, size, weight, italic == litehtml::fontStyleItalic);
    QFont *font = FontCache::getInstance()->getFont(faceName);
    if (!font) {
      qDebug() << "unsupported font " << faceName;
      return FontCache::getInstance()->getFont(FontCache::getInstance()->getDefaultFontName());
    }
    font->setUnderline(decoration & litehtml::font_decoration_underline);
    font->setOverline(decoration & litehtml::font_decoration_overline);
    font->setStrikeOut(decoration & litehtml::font_decoration_linethrough);
    QFontMetrics metrics(*font);
    fm->height = metrics.height();//metrics.ascent() + metrics.descent() + 2;
    fm->ascent = metrics.ascent();
    fm->descent = metrics.descent();
    fm->x_height = metrics.xHeight();//metrics.boundingRect("x").height();
    return font;//&QFont("Cousine");//
}

litehtml::tstring container_qt5::resolve_color(const litehtml::tstring& color) const
{
  struct custom_color
  {
    litehtml::tchar_t*	name;
    int					color_index;
  };
/*

    if (color == _t("Highlight"))
    {
        int iii = 0;
        iii++;
    }

    for (auto& clr : colors)
    {
        if (!t_strcasecmp(clr.name, color.c_str()))
        {
            litehtml::tchar_t  str_clr[20];
            DWORD rgb_color =  GetSysColor(clr.color_index);
#ifdef LITEHTML_UTF8
            StringCchPrintfA(str_clr, 20, "#%02X%02X%02X", GetRValue(rgb_color), GetGValue(rgb_color), GetBValue(rgb_color));
#else
            StringCchPrintf(str_clr, 20, L"#%02X%02X%02X", GetRValue(rgb_color), GetGValue(rgb_color), GetBValue(rgb_color));
#endif // LITEHTML_UTF8
            return std::move(litehtml::tstring(str_clr));
        }
    }*/
    return std::move(litehtml::tstring());
}


litehtmlWidget::litehtmlWidget(QWidget *parent)
    : QWidget(parent)
{
  setMouseTracking(true);

  // Within the body of a constructor or destructor,
  // there is a valid object of the class currently being constructed
  container = new container_qt5(this);
}

litehtmlWidget::~litehtmlWidget()
{

}

void litehtmlWidget::paintEvent(QPaintEvent *event)
{
    //container->setDrawArea(QRect(0, 0, width(), height()));
    //container->setSize(width(), height());

    QPainter painter(this);
    container->repaint(painter);
}

void litehtmlWidget::mouseMoveEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_mouse_over(event->x(), event->y(), event->x(), event->y(), redraw_boxes)) {
      repaint();
    }
}

void litehtmlWidget::mousePressEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_lbutton_down(event->x(), event->y(), event->x(), event->y(), redraw_boxes)) {
      repaint();
    }
}

void litehtmlWidget::mouseReleaseEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_lbutton_up(event->x(), event->y(), event->x(), event->y(), redraw_boxes)) {
      repaint();
    }
}

void litehtmlWidget::resizeEvent(QResizeEvent *event)
{
  container->_doc->media_changed();

  /*QRect	rc = rect( );

  container->_doc->render( rc.width() );
  container->_doc->render( rc.width() );*/
}

void litehtmlWidget::wheelEvent(QWheelEvent *event)
{

}
