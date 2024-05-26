int pins[6] = {LED_BUILTIN, D2, D3, D4, D5, D6};
String pinPaths[6] = {"/builtin", "/d2", "/d3", "/d4", "/d5", "/d6"};

int getPin(String path) {
    for (int i = 0; i < 6; i++) {
        if (path == pinPaths[i]) {
        return pins[i];
        }
    }
    return -1;
}

int getStatus(int value) {
    return value == 0 ? LOW : HIGH;
}