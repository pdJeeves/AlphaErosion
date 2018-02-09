#ifndef GLGLOBALS_H
#define GLGLOBALS_H

class QOpenGLFunctions_3_2_Core;

void AddRefGL();
void ReleaseGL(QOpenGLFunctions_3_2_Core*);
void bindVAO(QOpenGLFunctions_3_2_Core*);
uint32_t getWhiteTexture(QOpenGLFunctions_3_2_Core*);

#endif // GLGLOBALS_H
