/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2019  Pierre <pinaraf@pinaraf.info>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "container_qt5.h"
#include "types.h"
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>


container_qt5::container_qt5(QWidget *parent)
    : litehtml::document_container(), QWidget(parent)
{

}

container_qt5::~container_qt5()
{

}

void container_qt5::set_document(std::shared_ptr< litehtml::document > doc)
{
    _doc = doc;
}

void container_qt5::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    qDebug() << __FUNCTION__ << width();
    _doc->render(width());
    _doc->draw(&painter, 0, 0, nullptr);
}

void container_qt5::get_language(litehtml::tstring& language, litehtml::tstring& culture) const
{
    qDebug() << "get_language";
}

void container_qt5::get_media_features(litehtml::media_features& media) const
{
    qDebug() << "get_media_features";
    media.width = width();
    media.height = height();
    media.device_width = width();
    media.device_height = height();
    media.type = litehtml::media_type_screen;
    
    qDebug() << "=> " << media.width << "x" << media.height;
}

std::shared_ptr< litehtml::element > container_qt5::create_element(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr< litehtml::document >& doc)
{
    qDebug() << __FUNCTION__ << " this one can be 0";
    return 0;
}

void container_qt5::get_client_rect(litehtml::position& client) const
{
    qDebug() << "get_client_rect";
    // No scroll yet
    client.move_to(0, 0);
    client.width = width();
    client.height = height();
    qDebug() << "==> " << client.width << "x" << client.height;
}

void container_qt5::del_clip()
{
    qDebug() << "del_clip";
}

void container_qt5::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y)
{
    qDebug() << "set_clip";
}

void container_qt5::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
    qDebug() << "import_css";
}

void container_qt5::transform_text(litehtml::tstring& text, litehtml::text_transform tt)
{
    qDebug() << "transform_text";
}

void container_qt5::set_cursor(const litehtml::tchar_t* cursor)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::link(const std::shared_ptr< litehtml::document >& doc, const litehtml::element::ptr& el)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::set_base_url(const litehtml::tchar_t* base_url)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::set_caption(const litehtml::tchar_t* caption)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    qDebug() << __FUNCTION__ << " for root = " << root;
}

void container_qt5::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    qDebug() << __FUNCTION__;
}

const litehtml::tchar_t* container_qt5::get_default_font_name() const
{
    qDebug() << __FUNCTION__;
    return "Noto Sans";
}

int container_qt5::get_default_font_size() const
{
    qDebug() << __FUNCTION__;
    return 12;
}

int container_qt5::pt_to_px(int pt)
{
    qDebug() << __FUNCTION__;
}

void container_qt5::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    qDebug() << __FUNCTION__;
    QPainter *painter = (QPainter *) hdc;
    QFont *font = (QFont *) hFont;
    painter->setFont(*font);
    painter->setBrush(QColor(color.red, color.green, color.blue, color.alpha));
    QFontMetrics metrics(*font);
    
    qDebug() << "Paint " << text << " at " << pos.x << "x" << pos.y;
    painter->drawText(pos.x, pos.y/* + metrics.height()*/, text);
}

int container_qt5::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont)
{
    qDebug() << __FUNCTION__;
    QFont *font = (QFont *) hFont;
    QFontMetrics metrics(*font);
    QString txt(text);
    qDebug() << "For" << txt << metrics.boundingRect(txt);
    if (txt == " ") {
        return 42;
    }
    return metrics.boundingRect(txt).width();
}

void container_qt5::delete_font(litehtml::uint_ptr hFont)
{
    qDebug() << __FUNCTION__;
    QFont *font = (QFont *) hFont;
    delete(font);
}

litehtml::uint_ptr container_qt5::create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm)
{
    //TODO: decoration
    //TODO: fm
    qDebug() << __FUNCTION__ << " for " << faceName << size << weight;
    QFont *font = new QFont(faceName, size, weight, italic == litehtml::fontStyleItalic);
    QFontMetrics metrics(*font);
    fm->height = metrics.ascent() + metrics.descent() + 2;
    fm->ascent = metrics.ascent();
    fm->descent = metrics.descent();
    fm->x_height = metrics.boundingRect("x").height();
    return font;
}