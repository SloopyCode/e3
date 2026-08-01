# ui16
the core UI foundation for all sulfurLabs projects.

ui16 is a tree + container based UI framework for the [s4 desktop system](https://github.com/sulfurLabs/s4)

## example code
This is a simple example of ui16,

```c
#include <ui16.h>
#include <ui16buttons.h>

int main()
{
    // libdesktop stuff here...

    ui16_setRoot(
        style(
            width(fill),
            height(fill),
            bg(rgb(64, 64, 64))
        ),
        window_buffer,
        UI16DEMO_CONTENT_WIDTH,
        UI16DEMO_CONTENT_HEIGHT
    );

    ui16_container( /*this container is "root" as it is 
                      the first container, (main container) */
        style (
            layout(row),
            width(fill),
            height(fill)
        )
	  ) {
        ui16_container(
            style(
                width(percent(20)),
                bg(rgb(30,30,30))
            )
        ){
            ui16_button("button1");
        };

        ui16_container(
            style(
                width(fill),
                layout(column)
            )
        ) {
            ui16_label("label 1");
        };
  	}

    //draw loop if theres a own renderer
    // should use libdesktop then
}
```

if you use percentage in width, ui16 will automatically calculate the rest of the width for other containers.
if you want to specify a width independent from window size use pixels.