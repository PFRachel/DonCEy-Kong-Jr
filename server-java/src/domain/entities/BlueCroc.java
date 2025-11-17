
package domain.entities;
//producto concreto, también creado por la misma fábrica (CrocFactory).
public class BlueCroc implements GameObject {

    public int liana;
    public float vel;
    public float altura = 100;

    public BlueCroc(int liana, float vel) {
        this.liana = liana;
        this.vel = vel;
    }

    public void update(float dt) {
        altura -= vel * dt;
    }

    @Override
    public String toJson() {
        return "{\"tipo\":\"azul\",\"liana\":"+liana+",\"altura\":"+altura+"}";
    }
}
