package logic.factory;
//La fábrica abstracta
import domain.entities.GameObject;
//CREATOR ABSTRACTO.
public interface GameObjectFactory {
    GameObject create(String... params);
}
//Define cómo crean objetos TODAS las fábricas.
