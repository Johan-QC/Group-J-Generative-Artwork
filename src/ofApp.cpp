#include "ofApp.h"


namespace {
    const float W = 900;            // tamaño de la lámina (cuadrada)
    const float CX = W / 2;
    const float CY = W / 2;

    const float R_NET = 345;      // radio del círculo que contiene la red
    const float FRAME_H = 360;      // semilado del primer marco rectangular
    bool saveRequested = false;
    bool saveSvgRequested = false;

    // --- Loto central: rosa polar r = R*cos(8*a) -> 16 pétalos -----------------
    void drawLotus(float R, float angleOffset) {
        ofPolyline petals;
        for (float a = 0; a <= TWO_PI + 0.01; a += 0.005) {
            float r = R * cos(8 * a);
            petals.addVertex(CX + r * cos(a + angleOffset),
                CY + r * sin(a + angleOffset), 0);
        }
        petals.draw();
    }

    // --- Red de Indra: nodos en anillos concéntricos + líneas por distancia -----
    void drawIndraNet() {
        std::vector<ofVec2f> nodes;
        const int rings = 6;
        for (int i = 0; i < rings; i++) {
            float radius = 150 + 36 * i;
            int n = 12 + 6 * i;                 // más nodos cuanto más lejos
            float offset = i * 0.21;            // giro de cada anillo
            for (int k = 0; k < n; k++) {
                float a = offset + TWO_PI * k / n;
                nodes.push_back(ofVec2f(CX + radius * cos(a), CY + radius * sin(a)));
            }
        }

        const float minD = 24;                  // rango de resonancia
        const float maxD = 85;
        for (size_t i = 0; i < nodes.size(); i++) {
            for (size_t j = i + 1; j < nodes.size(); j++) {
                float d = nodes[i].distance(nodes[j]);
                if (d > minD && d < maxD) {
                    ofDrawLine(nodes[i].x, nodes[i].y, nodes[j].x, nodes[j].y);
                }
            }
        }

        // las "joyas": circulos vacios en cada nodo
        for (auto& p : nodes) {
            ofDrawCircle(p.x, p.y, 3);
        }
    }

    // --- Marcos del palacio: 6 rectángulos concéntricos --------------------------
    void drawFrames() {
        for (int i = 0; i < 6; i++) {
            float h = FRAME_H + i * 7;
            ofDrawRectangle(CX - h, CY - h, 2 * h, 2 * h);
        }
        // vacuidad: dos circulos que cierran la red
        ofDrawCircle(CX, CY, R_NET);
        ofDrawCircle(CX, CY, R_NET + 5);
    }

    // Dibuja el segmento (x0,y0)-(x1,y1) solo en la parte que queda FUERA del círculo de la red
    void strokeOutsideCircle(float x0, float y0, float x1, float y1) {
        const float step = 2;
        float len = ofDist(x0, y0, x1, y1);
        int steps = max(1, (int)(len / step));
        bool inRun = false;
        float ax = 0, ay = 0, bx = 0, by = 0;
        for (int s = 0; s <= steps; s++) {
            float t = (float)s / steps;
            float x = ofLerp(x0, x1, t);
            float y = ofLerp(y0, y1, t);
            if (ofDist(x, y, CX, CY) > R_NET + 8) {
                if (!inRun) { ax = x; ay = y; inRun = true; }
                bx = x; by = y;
            }
            else if (inRun) {
                ofDrawLine(ax, ay, bx, by);
                inRun = false;
            }
        }
        if (inRun) ofDrawLine(ax, ay, bx, by);
    }

    // --- Tramas en una esquina (sx, sy = +-1 indican la esquina) ----------------
    // Coordenadas locales (u,v) crecen desde la esquina hacia dentro.
    void hatchCorner(float sx, float sy) {
        float cornerX = CX + sx * FRAME_H;
        float cornerY = CY + sy * FRAME_H;
        auto toScreenX = [&](float u) { return cornerX - sx * u; };
        auto toScreenY = [&](float v) { return cornerY - sy * v; };

        // Familia 1: líneas u+v = d, más densas cerca de la esquina
        for (float d = 4; d < 260; d += 3 + d * 0.03) {
            strokeOutsideCircle(toScreenX(d), toScreenY(0), toScreenX(0), toScreenY(d));
        }

        // Familia 2: líneas u-v = e, solo cerca de la esquina (cross-hatching)
        const float L = 120;
        for (float e = -L; e <= L; e += 8) {
            float u0 = (e >= 0) ? e : 0;
            float v0 = (e >= 0) ? 0 : -e;
            float u1 = (L + e) / 2;
            float v1 = (L - e) / 2;
            strokeOutsideCircle(toScreenX(u0), toScreenY(v0), toScreenX(u1), toScreenY(v1));
        }
    }
}

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetWindowShape(W, W);
    ofSetFrameRate(10);          // imagen estática: no hace falta redibujar rápido
    ofSetLineWidth(1);
    ofSetCircleResolution(100);
    ofNoFill();                  // nada relleno: restricción del plotter
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(255);


    if (saveSvgRequested) {
        ofBeginSaveScreenAsSVG("pratityasamutpada.svg");
    }

    ofSetColor(0);
    ofNoFill();

    // 1. Núcleo: loto de 16 pétalos (dos capas) + círculos
    drawLotus(120, 0);
    drawLotus(80, PI / 16);
    ofDrawCircle(CX, CY, 125);
    ofDrawCircle(CX, CY, 130);
    ofDrawCircle(CX, CY, 30);

    // 2. La red de Indra
    drawIndraNet();

    // 3. Marcos y círculo de vacuidad
    drawFrames();

    // 4. Tramas en las cuatro esquinas
    hatchCorner(1, 1);
    hatchCorner(-1, 1);
    hatchCorner(1, -1);
    hatchCorner(-1, -1);

    if (saveSvgRequested) {
        ofEndSaveScreenAsSVG();
        saveSvgRequested = false;
        ofLog() << "SVG guardado: pratityasamutpada.svg";
    }

    if (saveRequested) {
        ofSaveScreen("pratityasamutpada.png");
        saveRequested = false;
    }
}

void ofApp::keyPressed(int key) {
    if (key == 's') saveRequested = true;
    if (key == 'v') saveSvgRequested = true;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}