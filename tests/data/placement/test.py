#!/usr/bin/env python
# -*- coding: utf-8 -*-

import mapnik2
import cairo
import sys
import os.path

dirname = os.path.dirname(sys.argv[0])

widths = [ 800, 600, 400, 300, 250, 200, 150, 100]
filenames = ["list", "simple"]
for filename in filenames:
    for width in widths:
	n = mapnik2.Map(width, 100)
	mapnik2.load_map(n, os.path.join(dirname, "%s.xml" % filename), False)
	bbox = mapnik2.Box2d(-0.05, -0.01, 0.95, 0.01)
	n.zoom_to_box(bbox)
	mapnik2.render_to_file(n, '%s-%d.png' % (filename, width))
    mapnik2.save_map(n, "%s-out.xml" % filename)
