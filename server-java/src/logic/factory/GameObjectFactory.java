package logic.factory;
//La fábrica abstracta
import domain.entities.GameObject;
import domain.Liana;
import java.util.List;

//CREATOR OBJETOS.
public abstract class GameObjectFactory {

    protected List<Liana> lianas;
    
    public GameObjectFactory(List<Liana> lianas) {
        this.lianas = lianas;
    }

    public abstract GameObject create();
    
}

//Define cómo crean objetos TODAS las fábricas.
