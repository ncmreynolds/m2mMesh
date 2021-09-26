# Blink

This sketch should blink the onboard LED of most development boards and once the mesh is joined and synced they will flash in sync.

It demonstrates use of the following principles...

Starting the mesh with 

```
begin();
```

Checking the mesh has formed with...

```
if(mesh.joined() == true()) {
...
}
```

Using time from the mesh instead of millis()...

```
if(mesh.syncedMillis() - lastBlink > blinkInterval) {
...
}
```

The logic is slightly more complicated than the original Arduino Blink sketch as it cannot use delay() and syncs the LEDs but is not complicated.