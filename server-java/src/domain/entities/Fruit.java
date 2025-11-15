package domain.entities;
//producto concreto creado por la FruitFactory.
public class Fruit implements GameObject {
//fruta con puntos 
    public int liana;
    public float altura;
    public int puntos;

    public Fruit(int liana, float altura, int puntos) {
        this.liana = liana;
        this.altura = altura;
        this.puntos = puntos;
    }

    @Override
    public String toJson() {
        return "{\"liana\":"+liana+",\"altura\":"+altura+",\"puntos\":"+puntos+"}";
    }
}
