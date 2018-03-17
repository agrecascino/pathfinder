#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <cmath>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
struct Line {
    glm::vec2 v1, v2;

    Line split() {
        Line x;
        x.v1 = v1;
        x.v2 = v2/2.0f;
        return x;
    }
};

struct Point {
    int x, y;
};

bool onseg(Point p, Point q, Point r) {
    if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
            q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
        return true;
    return false;
}

enum Orientation {
    Colinear = 0,
    Clockwise,
    Counterclockwise
};

Orientation orient(Point p, Point q, Point r) {
    int val = (q.y - p.y) * (r.x - q.x) -
            (q.x - p.x) * (r.y - q.y);
    if(val == 0) return Colinear;

    return (val > 0) ? Clockwise : Counterclockwise;
}

bool intersection(Line a, Line b) {
    Point a1;
    a1.x = a.v1.x;
    a1.y = a.v1.y;
    Point a2;
    a2.x = a.v2.x;
    a2.y = a.v1.y;
    Point b1;
    b1.x = b.v1.x;
    b1.y = b.v1.y;
    Point b2;
    b2.x = b.v2.x;
    b2.y = b.v2.y;
    Orientation o1 = orient(a1, a2, b1);
    Orientation o2 = orient(a1, a2, b2);
    Orientation o3 = orient(b1, b2, a1);
    Orientation o4 = orient(b1, b2, a2);
    if(o1 != o2 && o3 != o4)
        return true;
    return false;
}

struct RTriangle {
    Line oppleg;
    float angle;
    Line hypo(){
        Line ret;
        float v1a = oppleg.v2.x - oppleg.v1.x;
        glm::vec2 v = oppleg.v2 - oppleg.v1;

        float aaaaaaaa = v.length()/tan(angle);
        float angle = acos(v1a/v.length());
        float angle2 = 3.14159 - angle;
        float yheight = aaaaaaaa/sin(angle2);
        float v1b = aaaaaaaa/cos(angle2);
        ret.v1 = oppleg.v1;
        ret.v2 = glm::vec2(oppleg.v2.x + v1b, oppleg.v2.y + yheight);
        return ret;
    }
    RTriangle complement() {
        Line opplegn;
        opplegn.v1 = oppleg.v2;
        opplegn.v2 = oppleg.v2 + (oppleg.v2 - oppleg.v1);
        RTriangle t;
        t.angle = angle;
        t.oppleg = opplegn;
        return t;
    }
};

std::vector<RTriangle> pathfind(std::vector<Line> lines, Line hyp) {
    //Test if the path is unblocked.
    bool blocked = false;
    for(Line &l : lines) {
        if(intersection(l, hyp)) {
            blocked = true;
        }
    }
    if(!blocked) {
        RTriangle t;
        t.oppleg = hyp;
        t.angle = 0;
        std::vector<RTriangle> v;
        v.push_back(t);
        return v;
    }
    RTriangle tri;
    tri.oppleg = hyp.split();
    tri.angle = (1/9.0)*3.14159;
    while(tri.angle < 1.57) {
        blocked = false;
        for(Line &l : lines) {
            if(intersection(l, tri.hypo())) {
                blocked |= true;
            }
        }
        for(Line &l : lines) {
            if(intersection(l, tri.complement().hypo())) {
                blocked |= true;
            }
        }
        if(!blocked){
            break;
        }
        tri.angle += 3.159/4.0;
    }
    if(blocked){
        return std::vector<RTriangle>();
    }
    std::vector<RTriangle> v;
    v.push_back(tri);
    v.push_back(tri.complement());
    return v;
}

int main(void) {
    Display *d;
    Window w;
    XEvent e;
    char *msg = "File";
    int s;
    std::vector<Line> lines;

    d = XOpenDisplay(NULL);
    if (d == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    s = DefaultScreen(d);


    XColor c;
    c.red = 212*256;
    c.green = 208*256;
    c.blue = 200*256;
    c.flags = DoRed | DoGreen | DoBlue;
    XFontStruct* font_info;
    char* font_name = "*-helvetica-*-12-*";
    font_info = XLoadQueryFont(d,font_name);
    //if (!font_info) {
    //    fprintf(stderr, "XLoadQueryFont: failed loading font '%s'\n", font_name);
    //}
    XAllocColor(d,XDefaultColormap(d,s),&c);
    GC gc;
    XGCValues values;
    values.cap_style = CapButt;
    values.join_style = JoinBevel;
    unsigned long valuemask = GCCapStyle | GCJoinStyle;
    w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 640, 480, 1,
                            BlackPixel(d,s), c.pixel);
    XGrabPointer(d, w, False, ButtonPressMask | ButtonReleaseMask, GrabModeAsync,
                 GrabModeAsync, None, None, CurrentTime);
    XSelectInput(d, w, ExposureMask | ButtonPressMask | ButtonReleaseMask |KeyPressMask | StructureNotifyMask);
    XMapWindow(d, w);
    gc = XCreateGC(d,w,valuemask,&values);
    XSetBackground(d,gc,c.pixel);
    XSetForeground(d,gc,BlackPixel(d,s));
    XSetFillStyle(d, gc, FillSolid);
    XSetLineAttributes(d, gc, 2, LineSolid, CapRound, JoinRound);
    //XSetFont(d,gc,font_info->fid);
    Line l1;
    Line l2;
    l1.v1.x  = 20;
    l1.v2.x  = 80;
    l1.v1.y  = 80;
    l1.v2.y  = 80;
    l2.v1.x  = 40;
    l2.v1.y =  40;
    int width = 640;
    int height = 480;
    while (1) {
        XNextEvent(d, &e);
        if(e.type == ConfigureNotify) {
            width = e.xconfigure.width;
            height = e.xconfigure.height;
        }
        if(e.type == ButtonPress)
        {
            XClearWindow(d, w);
            l2.v2.x = e.xbutton.x;
            l2.v2.y = e.xbutton.y;
            if(intersection(l1, l2)) {
                XSetForeground(d, gc, BlackPixel(d,s));
                std::vector<Line> l;
                l.push_back(l1);
                std::vector<RTriangle> t = pathfind(l, l2);
                for(RTriangle tri : t) {
                    XSetForeground(d, gc, BlackPixel(d,s));
                    XDrawLine(d, w, gc, tri.hypo().v1.x,tri.hypo().v1.y, tri.hypo().v2.x, tri.hypo().v2.y);
                    XSetForeground(d, gc, WhitePixel(d,s));
                    XDrawLine(d, w, gc, tri.oppleg.v1.x,tri.oppleg.v1.y, tri.oppleg.v2.x, tri.oppleg.v2.y);
                }
            } else {
                XSetForeground(d, gc, BlackPixel(d,s));
                XDrawLine(d, w, gc, l2.v1.x, l2.v1.y, l2.v2.x, l2.v2.y);
            }
            XSetForeground(d, gc, BlackPixel(d,s));
            XDrawLine(d, w, gc, l1.v1.x, l1.v1.y, l1.v2.x, l1.v2.y);

            //XFillRectangle(d, w, gc, 0, 0, width, height);
            XDrawString(d, w, gc, 10, 20, msg, strlen(msg));
            XDrawString(d, w, gc, 50, 20, "", 0);
        }

        if (e.type == Expose) {
            XDrawLine(d, w, gc, l1.v1.x, l1.v1.y, l1.v2.x, l1.v2.y);
            //XFillRectangle(d, w, gc, 0, 0, width, height);
            XDrawString(d, w, gc, 10, 20, msg, strlen(msg));
            XDrawString(d, w, gc, 50, 20, "", 0);
        }
        if (e.type == KeyPress)
            break;
        XFlush(d);
    }

    XCloseDisplay(d);
    return 0;
}
