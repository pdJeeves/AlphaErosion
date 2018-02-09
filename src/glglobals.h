#ifndef GLGLOBALS_H
#define GLGLOBALS_H

class ViewWidget;

void AddRefGL();
void ReleaseGL(ViewWidget*);
void bindVAO(ViewWidget*);
uint32_t getWhiteTexture(ViewWidget*);

#endif // GLGLOBALS_H
