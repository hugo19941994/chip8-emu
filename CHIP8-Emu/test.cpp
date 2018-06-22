#include "./catch/catch.hpp"

TEST_CASE("Pushing ints to pelVector", "[pelVector]") {
    pel::Vector<int> v;
    v.popBack();
    REQUIRE(v.empty() == true);
}

TEST_CASE("Pushing Owners to pelVector", "[pelVector]") {
    v.popBack();
    REQUIRE(v.empty() == true);
}

TEST_CASE("Crear Owner completo", "[Owner]") {
    REQUIRE(o1.getNegos().at(0)->getNumeroPlazas() == 10);
}

TEST_CASE("Leer/Escribir en archivo el historial", "[entradaHistorial]") {
    REQUIRE(v2.at(1).getOwner() == "pepe");
}
