#ifndef LH_BORDERS_H
#define LH_BORDERS_H

#include "css_length.h"
#include "types.h"

namespace litehtml
{
	struct css_border
	{
		css_length		width;
		border_style	style;
		web_color		color;

		css_border()
		{
			style = border_style_none;
		}

		css_border(const css_border& val)
		{
			width	= val.width;
			style	= val.style;
			color	= val.color;
		}

		css_border& operator=(const css_border& val)
		{
			width	= val.width;
			style	= val.style;
			color	= val.color;
			return *this;
		}
	};

	struct border
	{
		int				width;
		border_style	style;
		web_color		color;

		border()
		{
			width = 0;
		}
		border(const border& val)
		{
			width = val.width;
			style = val.style;
			color = val.color;
		}
		border(const css_border& val)
		{
			width = (int) val.width.val();
			style = val.style;
			color = val.color;
		}
		border& operator=(const border& val)
		{
			width = val.width;
			style = val.style;
			color = val.color;
			return *this;
		}
		border& operator=(const css_border& val)
		{
			width = (int) val.width.val();
			style = val.style;
			color = val.color;
			return *this;
		}
	};

	struct border_radiuses
	{
		int	top_left_x;
		int	top_left_y;

		int	top_right_x;
		int	top_right_y;

		int	bottom_right_x;
		int	bottom_right_y;

		int	bottom_left_x;
		int	bottom_left_y;

		border_radiuses()
		{
			top_left_x = 0;
			top_left_y = 0;
			top_right_x = 0;
			top_right_y = 0;
			bottom_right_x = 0;
			bottom_right_y = 0;
			bottom_left_x = 0;
			bottom_left_y = 0;
		}
		border_radiuses(const border_radiuses& val)
		{
			top_left_x = val.top_left_x;
			top_left_y = val.top_left_y;
			top_right_x = val.top_right_x;
			top_right_y = val.top_right_y;
			bottom_right_x = val.bottom_right_x;
			bottom_right_y = val.bottom_right_y;
			bottom_left_x = val.bottom_left_x;
			bottom_left_y = val.bottom_left_y;
		}
		border_radiuses& operator = (const border_radiuses& val)
		{
			top_left_x = val.top_left_x;
			top_left_y = val.top_left_y;
			top_right_x = val.top_right_x;
			top_right_y = val.top_right_y;
			bottom_right_x = val.bottom_right_x;
			bottom_right_y = val.bottom_right_y;
			bottom_left_x = val.bottom_left_x;
			bottom_left_y = val.bottom_left_y;
			return *this;
		}
		void operator += (const margins& mg)
		{
			top_left_x += mg.left;
			top_left_y += mg.top;
			top_right_x += mg.right;
			top_right_y += mg.top;
			bottom_right_x += mg.right;
			bottom_right_y += mg.bottom;
			bottom_left_x += mg.left;
			bottom_left_y += mg.bottom;
			fix_values();
		}
		void operator -= (const margins& mg)
		{
			top_left_x -= mg.left;
			top_left_y -= mg.top;
			top_right_x -= mg.right;
			top_right_y -= mg.top;
			bottom_right_x -= mg.right;
			bottom_right_y -= mg.bottom;
			bottom_left_x -= mg.left;
			bottom_left_y -= mg.bottom;
			fix_values();
		}
		void fix_values()
		{
			if (top_left_x < 0)	top_left_x = 0;
			if (top_left_y < 0)	top_left_y = 0;
			if (top_right_x < 0) top_right_x = 0;
			if (bottom_right_x < 0) bottom_right_x = 0;
			if (bottom_right_y < 0) bottom_right_y = 0;
			if (bottom_left_x < 0) bottom_left_x = 0;
			if (bottom_left_y < 0) bottom_left_y = 0;
		}
	};

	struct css_border_radius
	{
		css_length	top_left_x;
		css_length	top_left_y;

		css_length	top_right_x;
		css_length	top_right_y;

		css_length	bottom_right_x;
		css_length	bottom_right_y;

		css_length	bottom_left_x;
		css_length	bottom_left_y;

		css_border_radius()
		{

		}

		css_border_radius(const css_border_radius& val)
		{
			top_left_x		= val.top_left_x;
			top_left_y		= val.top_left_y;
			top_right_x		= val.top_right_x;
			top_right_y		= val.top_right_y;
			bottom_left_x	= val.bottom_left_x;
			bottom_left_y	= val.bottom_left_y;
			bottom_right_x	= val.bottom_right_x;
			bottom_right_y	= val.bottom_right_y;
		}

		css_border_radius& operator=(const css_border_radius& val)
		{
			top_left_x		= val.top_left_x;
			top_left_y		= val.top_left_y;
			top_right_x		= val.top_right_x;
			top_right_y		= val.top_right_y;
			bottom_left_x	= val.bottom_left_x;
			bottom_left_y	= val.bottom_left_y;
			bottom_right_x	= val.bottom_right_x;
			bottom_right_y	= val.bottom_right_y;
			return *this;
		}
		border_radiuses calc_percents(int width, int height)
		{
			border_radiuses ret;
			ret.bottom_left_x = bottom_left_x.calc_percent(width);
			ret.bottom_left_y = bottom_left_y.calc_percent(height);
			ret.top_left_x = top_left_x.calc_percent(width);
			ret.top_left_y = top_left_y.calc_percent(height);
			ret.top_right_x = top_right_x.calc_percent(width);
			ret.top_right_y = top_right_y.calc_percent(height);
			ret.bottom_right_x = bottom_right_x.calc_percent(width);
			ret.bottom_right_y = bottom_right_y.calc_percent(height);
			return ret;
		}
	};

  struct borders_image
  {
    tstring				image_path;
    tstring				baseurl;
    size					image_size;
    tstring				css_prop;

    // TODO: slice sides https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/css/CSSStyleSelector.cpp#L6163
    int		slice;

    int		width;
    int		outset;
    background_repeat		repeat;
  };

	struct css_borders
	{
		css_border			left;
		css_border			top;
		css_border			right;
		css_border			bottom;
		css_border_radius	radius;
    borders_image image;

		css_borders()
		{

		}

		css_borders(const css_borders& val)
		{
			left	= val.left;
			right	= val.right;
			top		= val.top;
			bottom	= val.bottom;
			radius	= val.radius;
      image	= val.image;
		}

		css_borders& operator=(const css_borders& val)
		{
			left	= val.left;
			right	= val.right;
			top		= val.top;
			bottom	= val.bottom;
			radius	= val.radius;
      image	= val.image;
			return *this;
		}
  };

	struct borders
	{
		border			left;
		border			top;
		border			right;
		border			bottom;
    border_radiuses	radius;
    borders_image image;

		borders()
		{

		}

		borders(const borders& val)
		{
			left = val.left;
			right = val.right;
			top = val.top;
			bottom = val.bottom;
			radius = val.radius;
      image = val.image;
		}

		borders(const css_borders& val)
		{
			left = val.left;
			right = val.right;
			top = val.top;
			bottom = val.bottom;
      image = val.image;
		}

		borders& operator=(const borders& val)
		{
			left = val.left;
			right = val.right;
			top = val.top;
			bottom = val.bottom;
			radius = val.radius;
			return *this;
		}

		borders& operator=(const css_borders& val)
		{
			left = val.left;
			right = val.right;
			top = val.top;
			bottom = val.bottom;
			return *this;
		}
	};
}

#endif  // LH_BORDERS_H
