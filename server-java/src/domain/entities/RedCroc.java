package domain.entities;
//clase concreta que representa un cocodrilo rojo.
public class RedCroc implements GameObject {
//producto concreto creado por la fábrica de cocodrilos.
    public int liana;
    public float vel;
    public float altura = 0;
    public int dir = 1;//indica si sube o baja

    public RedCroc(int liana, float vel) {
        this.liana = liana;
        this.vel = vel;
    }

    public void update(float dt) {//controla movimiento.
        altura += vel * dt * dir;
        if (altura >= 100 || altura <= 0) dir *= -1;
    }

    @Override
    public String toJson() {
        return "{\"tipo\":\"rojo\",\"liana\":"+liana+",\"altura\":"+altura+"}";
    }
}
