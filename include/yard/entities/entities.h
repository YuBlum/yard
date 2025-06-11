#ifndef __ENTITIES_H__
#define __ENTITIES_H__

struct entities_layout /*{
  uint32_t foo_amount;
}*/;

bool entities_make(void);
bool entities_layout_set(const struct entities_layout *layout);
void entities_update(float dt);
void entities_render(void);

#endif/*__ENTITIES_H__*/
